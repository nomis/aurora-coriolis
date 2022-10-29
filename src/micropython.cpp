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

#include <uuid/console.h>

extern "C" {
	#include <py/compile.h>
	#include <py/gc.h>
	#include <py/lexer.h>
	#include <py/mperrno.h>
	#include <py/mphal.h>
	#include <py/nlr.h>
	#include <py/runtime.h>
	#include <py/stackctrl.h>
	#include <shared/runtime/pyexec.h>
}

using uuid::console::Shell;

static const char __pstr__logger_name[] __attribute__((__aligned__(sizeof(int)))) PROGMEM = "mpy";

namespace aurcor {

uuid::log::Logger MicroPython::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::USER};
__thread MicroPython *MicroPython::self_ = nullptr;

MicroPython::MicroPython(run_function f) : main_(f) {
	heap_ = (uint8_t *)::heap_caps_malloc(HEAP_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
	if (!heap_)
		logger_.emerg(F("[%p] Unable to allocate heap"), this);
}

MicroPython::~MicroPython() {
	::free(heap_);
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

	logger_.trace(F("[%p] MicroPython initialising"), this);

	if (!::setjmp(abort_)) {
		where_ = F("mp_stack_ctrl_init");
		mp_stack_ctrl_init();
	} else {
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = F("gc_init");
		gc_init(heap_, heap_ + HEAP_SIZE);
	} else {
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = F("mp_init");
		mp_init();

		if (!::setjmp(abort_)) {
			logger_.trace(F("[%p] MicroPython running"), this);

			where_ = F("main_");
			main_();

			logger_.trace(F("[%p] MicroPython shutdown"), this);
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
	logger_.trace(F("[%p] MicroPython finished"), this);
	self_ = nullptr;
	running_ = false;
}

void MicroPython::stop() {
	if (!started_ || stopped_)
		return;

	if (running_) {
		logger_.trace(F("[%p] Stopping thread"), this);
		running_ = false;

		shutdown();
	}

	if (thread_.joinable()) {
		logger_.trace(F("[%p] Waiting for thread to stop"), this);
		thread_.join();
		logger_.trace(F("[%p] Thread stopped"), this);
	}

	stopped_ = true;
}

MicroPythonShell::MicroPythonShell()
		: MicroPython([] { ::pyexec_friendly_repl(); }) {
}

void MicroPythonShell::start(Shell &shell) {
	auto self = shared_from_this();

	if (!heap_available()) {
		shell.printfln(F("No heap available"));
		return;
	}

	logger_.trace(F("[%p] Starting thread"), this);

	MicroPython::start();

	shell.block_with([self] (Shell &shell, bool stop) -> bool {
		return self->shell_foreground(shell, stop);
	});
}

bool MicroPythonShell::shell_foreground(Shell &shell, bool stop_) {
	if (running() && shell.available() && stdin_.write_available())
		stdin_.write(shell.read());

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

void MicroPythonShell::shutdown() {
	stdin_.stop();
	stdout_.stop();
}

} // namespace aurcor

using aurcor::MicroPython;

extern "C" void nlr_jump_fail(void *val) {
	MicroPython::self_->nlr_jump_fail(val);
}

void aurcor::MicroPython::nlr_jump_fail(void *val) {
	logger_.alert(F("[%p] MicroPython failed in %S()"), this, where_);
	::longjmp(abort_, 1);
}

extern "C" mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
	return MicroPython::self_->mp_lexer_new_from_file(filename);
}

mp_lexer_t *aurcor::MicroPython::mp_lexer_new_from_file(const char *filename) {
	mp_raise_OSError(MP_ENOENT);
}

extern "C" int mp_hal_stdin_rx_chr(void) {
	return MicroPython::self_->mp_hal_stdin_rx_chr();
}

int aurcor::MicroPythonShell::mp_hal_stdin_rx_chr(void) {
	if (!running())
		return -1;

	int c = stdin_.read(false);
	if (c != -1 || !running())
		return c;

	mp_handle_pending(true);
	MP_THREAD_GIL_EXIT();

	if (running())
		c = stdin_.read(true);

	MP_THREAD_GIL_ENTER();
	return c;
}

extern "C" void mp_hal_stdout_tx_strn(const char *str, size_t len) {
	MicroPython::self_->mp_hal_stdout_tx_strn(reinterpret_cast<const uint8_t *>(str), len);
}

void aurcor::MicroPythonShell::mp_hal_stdout_tx_strn(const uint8_t *str, size_t len) {
	if (!running())
		mp_raise_OSError(MP_ENOEXEC);

	while (len > 0) {
		size_t written = stdout_.write(str, len, false);

		if (!running())
			mp_raise_OSError(MP_ENOEXEC);

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
			mp_raise_OSError(MP_ENOEXEC);

		str += written;
		len -= written;
	}
}
