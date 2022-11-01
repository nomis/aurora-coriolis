/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022  Simon Arlott
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "aurcor/micropython.h"

#include <Arduino.h>

#include <algorithm>
#include <csetjmp>
#include <cstring>
#include <memory>
#include <string>
#include <thread>

#include <uuid/console.h>

extern "C" {
	#include <py/compile.h>
	#include <py/gc.h>
	#include <py/lexer.h>
	#include <py/mperrno.h>
	#include <py/mphal.h>
	#include <py/mpstate.h>
	#include <py/nlr.h>
	#include <py/runtime.h>
	#include <py/stackctrl.h>
	#include <shared/runtime/interrupt_char.h>
	#include <shared/runtime/pyexec.h>
}

using uuid::console::Shell;

static const char __pstr__logger_name[] __attribute__((__aligned__(sizeof(int)))) PROGMEM = "mpy";

namespace aurcor {

uuid::log::Logger MicroPython::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::USER};

__thread MicroPython *MicroPython::self_{nullptr};

std::shared_ptr<Heaps> MicroPython::heaps_ = std::make_shared<Heaps>(
	MicroPython::HEAP_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
std::shared_ptr<Heaps> MicroPython::pystacks_ = std::make_shared<Heaps>(
	MicroPython::PYSTACK_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

void MicroPython::setup(size_t heap_count) {
	heaps_->resize(heap_count);
	pystacks_->resize(heap_count);
}

MicroPython::MicroPython(const std::string &name)
		: name_(name), heap_(std::move(heaps_->allocate())),
		pystack_(std::move(pystacks_->allocate())) {
	system_exit_exc_.base.type = &mp_type_SystemExit;
	system_exit_exc_.traceback_alloc = 0;
	system_exit_exc_.traceback_len = 0;
	system_exit_exc_.traceback_data = NULL;
	system_exit_exc_.args = (mp_obj_tuple_t *)&mp_const_empty_tuple_obj;
}

void MicroPython::start() {
	if (started_)
		return;

	started_ = true;
	running_ = true;
	thread_ = std::thread{&MicroPython::running_thread, this};
}

void MicroPython::running_thread() {
	self_ = this;

	logger_.trace(F("[%s/%p] MicroPython initialising"), name_.c_str(), this);

	if (mp_state_init()) {
		logger_.alert(F("[%s/%p] MicroPython failed in mp_state_init()"), name_.c_str(), this);
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = F("mp_stack_ctrl_init");
		mp_stack_ctrl_init();
	} else {
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = F("gc_init");
		gc_init(heap_->begin(), heap_->end());
	} else {
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = F("mp_pystack_init");
		mp_pystack_init(pystack_->begin(), pystack_->end());
	} else {
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = F("mp_init");
		mp_init();

		{
			std::lock_guard<std::mutex> lock{state_mutex_};
			state_copy();
		}

		if (running_ && !::setjmp(abort_)) {
			logger_.trace(F("[%s/%p] MicroPython running"), name_.c_str(), this);

			where_ = F("main");
			main();

			logger_.trace(F("[%s/%p] MicroPython shutdown"), name_.c_str(), this);
		}

		{
			std::lock_guard<std::mutex> lock{state_mutex_};
			state_reset();
		}

		if (!::setjmp(abort_)) {
			where_ = F("gc_sweep_all");
			gc_sweep_all();
		}

		if (!::setjmp(abort_)) {
			where_ = F("mp_deinit");
			mp_deinit();
		}
	} else {
		if (!::setjmp(abort_)) {
			where_ = F("gc_sweep_all");
			gc_sweep_all();
		}
	}

done:
	mp_state_free();

	logger_.trace(F("[%s/%p] MicroPython finished"), name_.c_str(), this);
	self_ = nullptr;
	running_ = false;
}

void MicroPython::state_copy() {
	state_ctx_ = mp_state_ctx;
	exec_system_exit_ = &pyexec_system_exit;
}

void MicroPython::state_reset() {
	state_ctx_ = nullptr;
	exec_system_exit_ = nullptr;
}

void MicroPython::force_exit() {
	AccessState access{*this};

	if (running_) {
		running_ = false;

		if (exec_system_exit_)
			*exec_system_exit_ = PYEXEC_FORCED_EXIT;

		if (access.enable()) {
			mp_sched_exception(MP_OBJ_FROM_PTR(&system_exit_exc_));
			access.disable();
		}

		shutdown();
	}
}

void MicroPython::abort() {
	pyexec_system_exit = PYEXEC_FORCED_EXIT;
	::nlr_raise(MP_OBJ_FROM_PTR(&system_exit_exc_));
}

void MicroPython::stop() {
	if (!started_ || stopped_)
		return;

	if (running_) {
		logger_.trace(F("[%s/%p] Stopping thread"), name_.c_str(), this);
		force_exit();
	}

	if (thread_.joinable()) {
		logger_.trace(F("[%s/%p] Waiting for thread to stop"), name_.c_str(), this);
		thread_.join();
		logger_.trace(F("[%s/%p] Thread stopped"), name_.c_str(), this);
	}

	stopped_ = true;
}
MicroPython::AccessState::AccessState(MicroPython &mp)
		: mp_(mp), state_lock_(mp.state_mutex_),
		atomic_section_(mp.atomic_section_mutex_, std::defer_lock) {
}

MicroPython::AccessState::~AccessState() {
	disable();
}

bool MicroPython::AccessState::enable() {
	if (!enabled_ && mp_.state_ctx_) {
		mp_state_ctx = mp_.state_ctx_;
		atomic_section_.lock();
		enabled_ = true;
		return true;
	} else {
		return false;
	}
}

void MicroPython::AccessState::disable() {
	if (enabled_) {
		atomic_section_.unlock();
		mp_state_ctx = nullptr;
		enabled_ = false;
	}
}

MicroPythonShell::MicroPythonShell(const std::string &name)
		: MicroPython(name) {
}

void MicroPythonShell::start(Shell &shell) {
	auto self = shared_from_this();

	if (!heap_available()) {
		shell.printfln(F("No heap available"));
		return;
	}

	logger_.trace(F("[%s/%p] Starting thread"), name_.c_str(), this);

	MicroPython::start();

	shell.block_with([self] (Shell &shell, bool stop) -> bool {
		return self->shell_foreground(shell, stop);
	});
}

void MicroPythonShell::main() {
	::pyexec_friendly_repl();
}

bool MicroPythonShell::shell_foreground(Shell &shell, bool stop_) {
	if (running() && shell.available()) {
		if (stdin_.write_available()) {
			int c = shell.read();

			if (c == '\x1C') {
				// Quit (^\)
				force_exit();
			} else if (c != -1 && !interrupt_char(c)) {
				stdin_.write(c);
			}
		} else {
			int c = shell.peek();

			if (c == '\x1C') {
				// Quit (^\)
				force_exit();
				shell.read();
			} else if (c != -1 && interrupt_char(c)) {
				shell.read();
			}
		}
	}

	const uint8_t *buf;
	auto len = stdout_.read(buf, false);

	if (len > 0) {
		shell.write(buf, len);
		stdout_.take(len);
	}

	if (stop_ || (!running() && stdout_.read_available() == 0)) {
		stop();
		return true;
	}

	return false;
}

bool MicroPythonShell::interrupt_char(int c) {
	AccessState access{*this};

	if (running() && interrupt_char_ && *interrupt_char_ == c) {
		if (access.enable())
			mp_sched_keyboard_interrupt();

		return true;
	}

	return false;
}

void MicroPythonShell::shutdown() {
	stdin_.stop();
	stdout_.stop();
}

void MicroPythonShell::state_copy() {
	MicroPython::state_copy();

	interrupt_char_ = &mp_interrupt_char;
}

void MicroPythonShell::state_reset() {
	interrupt_char_ = nullptr;

	MicroPython::state_reset();
}

} // namespace aurcor

using aurcor::MicroPython;

extern "C" void nlr_jump_fail(void *val) {
	MicroPython::self_->nlr_jump_fail(val);
}

void aurcor::MicroPython::nlr_jump_fail(void *val) {
	if (running_) {
		logger_.alert(F("[%s/%p] MicroPython failed in %S(): %p"), name_.c_str(), this, where_, val);
	} else {
		logger_.trace(F("[%s/%p] MicroPython aborted in %S(): %p"), name_.c_str(), this, where_, val);
	}
	::longjmp(abort_, 1);
}

extern "C" mp_uint_t mp_hal_begin_atomic_section(void) {
	MicroPython::self_->mp_hal_begin_atomic_section();
	return 1;
}

void aurcor::MicroPython::mp_hal_begin_atomic_section() {
	atomic_section_mutex_.lock();
}

extern "C" void mp_hal_end_atomic_section() {
	MicroPython::self_->mp_hal_end_atomic_section();
}

void aurcor::MicroPython::mp_hal_end_atomic_section() {
	atomic_section_mutex_.unlock();
}

extern "C" mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
	return MicroPython::self_->mp_lexer_new_from_file(filename);
}

mp_lexer_t *aurcor::MicroPython::mp_lexer_new_from_file(const char *filename) {
	mp_raise_OSError(MP_ENOENT);
}

extern "C" int mp_hal_stdin_rx_chr(void) {
	// Return values must be in the uint8_t range (-1 is not special)
	return MicroPython::self_->mp_hal_stdin_rx_chr();
}

int aurcor::MicroPythonShell::mp_hal_stdin_rx_chr(void) {
	int c = -1;

	if (!running())
		goto done;

	c = stdin_.read(false);
	if (c != -1 || !running())
		goto done;

	mp_handle_pending(true);
	MP_THREAD_GIL_EXIT();

	if (running())
		c = stdin_.read(true);

	MP_THREAD_GIL_ENTER();

done:
	if (c == -1) {
		// Return values must be in the uint8_t range (-1 is not special)
		abort();
	}

	return c;
}

extern "C" void mp_hal_stdout_tx_strn(const char *str, size_t len) {
	MicroPython::self_->mp_hal_stdout_tx_strn(reinterpret_cast<const uint8_t *>(str), len);
}

void aurcor::MicroPythonShell::mp_hal_stdout_tx_strn(const uint8_t *str, size_t len) {
	if (!running())
		abort();

	while (len > 0) {
		size_t written = stdout_.write(str, len, false);

		if (!running())
			abort();

		if (written > 0) {
			str += written;
			len -= written;
			continue;
		}

		mp_handle_pending(true);
		MP_THREAD_GIL_EXIT();

		if (running())
			written = stdout_.write(str, len, true);

		MP_THREAD_GIL_ENTER();
		if (!running())
			abort();

		str += written;
		len -= written;
	}
}
