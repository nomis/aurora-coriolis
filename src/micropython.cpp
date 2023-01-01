/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022-2023  Simon Arlott
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

#if defined(ARDUINO_ARCH_ESP32)
# include <soc/cache_memory.h>
#endif

#include <algorithm>
#include <csetjmp>
#include <cstring>
#include <memory>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

extern "C" {
	#include <py/builtin.h>
	#include <py/compile.h>
	#include <py/gc.h>
	#include <py/lexer.h>
	#include <py/mperrno.h>
	#include <py/mphal.h>
	#include <py/mpstate.h>
	#include <py/nlr.h>
	#include <py/persistentcode.h>
	#include <py/runtime.h>
	#include <py/stackctrl.h>
	#include <shared/runtime/interrupt_char.h>
	#include <shared/runtime/pyexec.h>
}

#include <uuid/console.h>
#include <uuid/log.h>

#include "app/fs.h"
#include "app/gcc.h"
#include "app/util.h"
#include "aurcor/app.h"
#include "aurcor/io_buffer.h"
#include "aurcor/led_bus.h"
#include "aurcor/modaurcor.h"
#include "aurcor/modulogging.h"
#include "aurcor/memory_pool.h"
#include "aurcor/mp_print.h"
#include "aurcor/mp_reader.h"
#include "aurcor/util.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

using aurcor::micropython::LogPrint;
using aurcor::micropython::PlatformPrint;
using uuid::console::Shell;

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "mpy";

namespace aurcor {

uuid::log::Logger MicroPython::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::USER};

__thread MicroPython *MicroPython::self_{nullptr};

std::shared_ptr<MemoryPool> MicroPython::heaps_ = std::make_shared<MemoryPool>(
	MicroPython::HEAP_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
std::shared_ptr<MemoryPool> MicroPython::pystacks_ = std::make_shared<MemoryPool>(
	MicroPython::PYSTACK_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
std::shared_ptr<MemoryPool> MicroPython::ledbufs_ = std::make_shared<MemoryPool>(
	LEDBus::MAX_BYTES, MALLOC_CAP_DEFAULT | MALLOC_CAP_8BIT);

void MicroPython::setup(size_t pool_count) {
	heaps_->resize(pool_count);
	pystacks_->resize(pool_count);
	ledbufs_->resize(pool_count);
}

MicroPython::MicroPython(const std::string &name,
		std::shared_ptr<LEDBus> bus, std::shared_ptr<Preset> preset)
		: name_(name + "/" + bus->name()),
		heap_(std::move(heaps_->allocate())),
		pystack_(std::move(pystacks_->allocate())),
		ledbuf_(std::move(ledbufs_->allocate())),
		modaurcor_(ledbuf_.get(), std::move(bus), std::move(preset)) {
	system_exit_exc_.base.type = &mp_type_SystemExit;
	system_exit_exc_.traceback_alloc = 0;
	system_exit_exc_.traceback_len = 0;
	system_exit_exc_.traceback_data = NULL;
	system_exit_exc_.args = (mp_obj_tuple_t *)&mp_const_empty_tuple_obj;

	if (!memory_blocks_available()) {
		heap_.reset();
		pystack_.reset();
		ledbuf_.reset();
	}
}

bool MicroPython::start() {
	if (started_ || !memory_blocks_available() || stopped_)
		return false;

	logger_.trace(F("[%s] Starting thread"), name_.c_str());

	started_ = true;
	running_ = true;
	thread_ = std::thread{&MicroPython::running_thread, this};

	return true;
}

void MicroPython::running_thread() {
	std::lock_guard lock{active_};

	if (!running_)
		return;

	self_ = this;

	logger_.trace(F("[%s] MicroPython initialising"), name_.c_str());

	if (mp_state_init()) {
		logger_.alert(F("[%s] MicroPython failed in mp_state_init()"), name_.c_str());
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = "mp_stack_ctrl_init";
		mp_stack_ctrl_init();
	} else {
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = "mp_stack_set_limit";
		mp_stack_set_limit(TASK_STACK_LIMIT);
	} else {
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = "gc_init";
		gc_init(heap_->begin(), heap_->end());
	} else {
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = "mp_pystack_init";
		mp_pystack_init(pystack_->begin(), pystack_->end());
	} else {
		goto done;
	}

	if (!::setjmp(abort_)) {
		where_ = "mp_init";
		mp_init();

		{
			std::lock_guard lock{state_mutex_};
			state_copy();
		}

		if (running_ && !::setjmp(abort_)) {
			logger_.trace(F("[%s] MicroPython running"), name_.c_str());

			where_ = "main";
			main();

			logger_.trace(F("[%s] MicroPython shutdown"), name_.c_str());
		}

		{
			std::lock_guard lock{state_mutex_};
			state_reset();
		}

		if (!::setjmp(abort_)) {
			where_ = "gc_sweep_all";
			gc_sweep_all();
		}

		if (!::setjmp(abort_)) {
			where_ = "mp_deinit";
			mp_deinit();
		}
	} else {
		if (!::setjmp(abort_)) {
			where_ = "gc_sweep_all";
			gc_sweep_all();
		}
	}

done:
	mp_state_free();

	logger_.trace(F("[%s] MicroPython finished"), name_.c_str());
	self_ = nullptr;
	running_ = false;
}

void MicroPython::state_copy() {
#if MICROPY_INSTANCE_PER_THREAD
	state_ctx_ = mp_state_ctx_thread;
#endif
	exec_system_exit_ = &pyexec_system_exit;
}

void MicroPython::main() {
}

void MicroPython::state_reset() {
#if MICROPY_INSTANCE_PER_THREAD
	state_ctx_ = nullptr;
#endif
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

void MicroPython::shutdown() {
}

bool MicroPython::stop() {
	if (!started_) {
		stopped_ = true;
		return true;
	}

	stopping_ = true;

	if (running_) {
		logger_.trace(F("[%s] Stopping thread"), name_.c_str());
		force_exit();
	}

	if (thread_.joinable()) {
		std::unique_lock lock{active_, std::try_to_lock};

		if (lock.owns_lock()) {
			lock.unlock();
			thread_.join();
			heap_.reset();
			pystack_.reset();
			ledbuf_.reset();
			logger_.trace(F("[%s] Thread stopped"), name_.c_str());
		} else if (!stopped_) {
			logger_.trace(F("[%s] Waiting for thread to stop"), name_.c_str());
		}
	}

	stopped_ = true;
	return !thread_.joinable();
}

void MicroPython::cleanup() {
	if (started_ && !stopped_) {
		logger_.emerg(F("[%s] Still running in destructor"), name_.c_str());

		while (!stop()) {}
	}
}

std::string MicroPython::script_filename(const char *path) {
	std::string filename = DIRECTORY_NAME;

	filename.append(1, '/');
	filename.append(app::normalise_filename(path));

	return filename;
}

std::string MicroPython::log_prefix(const char type) {
	std::vector<char> prefix(1 + name_.size() + 3 + 1);

	::snprintf(prefix.data(), prefix.size(), "[%s] %c", name_.c_str(), type);

	return prefix.data();
}

void MicroPython::log_exception(mp_obj_t exc, uuid::log::Level level) {
	micropython_nlr_begin();

	std::string prefix = log_prefix('E');
	LogPrint print{logger_, level, prefix};

	mp_stack_set_limit(TASK_EXC_STACK_LIMIT);

	micropython_nlr_try();

	mp_obj_print_exception(print.context(), exc);

	micropython_nlr_finally();

	mp_stack_set_limit(TASK_STACK_LIMIT);

	micropython_nlr_end();
}

uuid::log::Level MicroPython::modulogging_effective_level() {
	return logger_.effective_level();
}

std::unique_ptr<micropython::Print> MicroPython::modulogging_print(uuid::log::Level level) {
	std::vector<char> prefix(1 + name_.size() + 3 + 1);

	::snprintf(prefix.data(), prefix.size(), "[%s] L", name_.c_str());

	return std::make_unique<LogPrint>(logger_, level, prefix.data());
}

MicroPython::AccessState::AccessState(MicroPython &mp)
		: mp_(mp), state_lock_(mp.state_mutex_),
		atomic_section_(mp.atomic_section_mutex_, std::defer_lock) {
}

MicroPython::AccessState::~AccessState() {
	disable();
}

bool MicroPython::AccessState::enable() {
	if (enabled_)
		return true;

#if MICROPY_INSTANCE_PER_THREAD
	if (!mp_.state_ctx_)
		return false;

	mp_state_ctx_thread = mp_.state_ctx_;
#endif

	atomic_section_.lock();
	enabled_ = true;
	return true;
}

void MicroPython::AccessState::disable() {
	if (enabled_) {
		atomic_section_.unlock();
#if MICROPY_INSTANCE_PER_THREAD
		mp_state_ctx_thread = nullptr;
#endif
		enabled_ = false;
	}
}

MicroPythonShell::MicroPythonShell(const std::string &name,
		std::shared_ptr<LEDBus> bus, std::shared_ptr<Preset> preset)
		: MicroPython(name, bus, preset) {
}

MicroPythonShell::~MicroPythonShell() {
	cleanup();
}

bool MicroPythonShell::start(Shell &shell) {
	if (!memory_blocks_available()) {
		shell.printfln(F("Out of memory"));
		return false;
	}

	if (!MicroPython::start()) {
		shell.printfln(F("Failed to start"));
		return false;
	}

	return true;
}

void MicroPythonShell::main() {
	while (1) {
		if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
			if (::pyexec_raw_repl())
				break;
		} else {
			if (::pyexec_friendly_repl())
				break;
		}
	}
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
	auto len = stdout_.read_available(buf, false);

	if (len > 0) {
		shell.write(buf, len);
		stdout_.read_consume(len);
	}

	if (stop_ || (!running() && stdout_.read_available() == 0)) {
		if (stop())
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

uuid::log::Level MicroPythonShell::modulogging_effective_level() {
	return uuid::log::Level::ALL;
}

std::unique_ptr<aurcor::micropython::Print> MicroPythonShell::modulogging_print(uuid::log::Level level) {
	return std::make_unique<PlatformPrint>(level);
}

MicroPythonFile::MicroPythonFile(const std::string &name,
		std::shared_ptr<LEDBus> bus, std::shared_ptr<Preset> preset)
		: MicroPython(name, bus, preset), name_(name), stdout_prefix_(log_prefix('O')),
		stdout_(logger_, uuid::log::Level::NOTICE, stdout_prefix_), log_prefix_(log_prefix('L')) {
}

MicroPythonFile::~MicroPythonFile() {
	cleanup();
}

std::vector<std::string> MicroPythonFile::scripts() {
	return list_filenames(DIRECTORY_NAME, FILENAME_EXT);
}

bool MicroPythonFile::exists(const char *name) {
	std::shared_lock file_lock{App::file_mutex()};
	return app::FS.exists(script_filename(filename_ext(name).c_str()).c_str());
}

std::string MicroPythonFile::filename_ext(const char *name) {
	std::string filename{name};
	filename.append(FILENAME_EXT);
	return filename;
}

void MicroPythonFile::main() {
	nlr_buf_t nlr;
	nlr.ret_val = nullptr;
	if (!nlr_push(&nlr)) {
		mp_module_context_t *context = m_new_obj(mp_module_context_t);
		context->module.globals = mp_globals_get();
		mp_compiled_module_t cm = mp_raw_code_load_file(filename_ext(name_.c_str()).c_str(), context);
		mp_obj_t module_fun = mp_make_function_from_raw_code(cm.rc, cm.context, NULL);

		mp_call_function_0(module_fun);
		mp_handle_pending(true);
		nlr_pop();
	} else {
		mp_handle_pending(false);
		log_exception(MP_OBJ_FROM_PTR(nlr.ret_val), stopping() ? uuid::log::TRACE : uuid::log::NOTICE);
	}
}

uuid::log::Level MicroPythonFile::modulogging_effective_level() {
	return logger_.effective_level();
}

std::unique_ptr<aurcor::micropython::Print> MicroPythonFile::modulogging_print(uuid::log::Level level) {
	return std::make_unique<LogPrint>(logger_, level, log_prefix_);
}

} // namespace aurcor

using aurcor::MicroPython;

extern "C" void nlr_jump_fail(void *val) {
	MicroPython::current().nlr_jump_fail(val);
}

void aurcor::MicroPython::nlr_jump_fail(void *val) {
	uuid::log::Level level = (running_ || in_nlr_fail_) ? uuid::log::Level::ALERT : uuid::log::Level::NOTICE;
	uintptr_t address = (uintptr_t)val;
	bool valid = false;

	logger_.log(level, logger_.facility(), F("[%s] MicroPython aborted in %s(): %p"),
		name_.c_str(), where_, val);

#if defined(ARDUINO_ARCH_ESP32)
	if (ADDRESS_IN_IRAM0(address)
			|| ADDRESS_IN_IRAM0_CACHE(address)
			|| ADDRESS_IN_IRAM1(address)
			|| ADDRESS_IN_DROM0(address)
			|| ADDRESS_IN_DRAM0(address)
			|| ADDRESS_IN_DRAM0_CACHE(address)
			|| ADDRESS_IN_DRAM1(address)
			|| (val >= heap_->begin() && val < heap_->end())
			|| (val >= pystack_->begin() && val < pystack_->end()))
		valid = true;
#elif defined(ENV_NATIVE)
	valid = true;
#endif

	if (val == nullptr || address == 0xBAAAAAAD || address == 0xBAADF00D)
		valid = false;

	if (valid && !in_nlr_fail_) {
		in_nlr_fail_ = true;
		log_exception(MP_OBJ_FROM_PTR(address), level);
		in_nlr_fail_ = false;
	}

	::longjmp(abort_, 1);
}

extern "C" mp_uint_t mp_hal_begin_atomic_section(void) {
	MicroPython::current().mp_hal_begin_atomic_section();
	return 1;
}

void aurcor::MicroPython::mp_hal_begin_atomic_section() {
	atomic_section_mutex_.lock();
}

extern "C" void mp_hal_end_atomic_section() {
	MicroPython::current().mp_hal_end_atomic_section();
}

void aurcor::MicroPython::mp_hal_end_atomic_section() {
	atomic_section_mutex_.unlock();
}

extern "C" ::mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    return mp_lexer_new(qstr_from_str(filename),
		aurcor::micropython::Reader::from_file(
			aurcor::MicroPython::script_filename(filename).c_str()));
}

extern "C" ::mp_import_stat_t mp_import_stat(const char *path) {
	std::shared_lock file_lock{aurcor::App::file_mutex()};
	auto file = app::FS.open(aurcor::MicroPython::script_filename(path).c_str());
	return !file ? MP_IMPORT_STAT_NO_EXIST
		: (file.isDirectory() ? MP_IMPORT_STAT_DIR : MP_IMPORT_STAT_FILE);
}

extern "C" void mp_reader_new_file(::mp_reader_t *reader, const char *filename) {
    *reader = aurcor::micropython::Reader::from_file(
		aurcor::MicroPython::script_filename(filename).c_str());
}

extern "C" int mp_hal_stdin_rx_chr(void) {
	// Return values must be in the uint8_t range (-1 is not special)
	return MicroPython::current().mp_hal_stdin_rx_chr();
}

int aurcor::MicroPython::mp_hal_stdin_rx_chr(void) {
	if (running_) {
		mp_raise_OSError(MP_ENODEV);
	} else {
		abort();
	}
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
	MicroPython::current().mp_hal_stdout_tx_strn(reinterpret_cast<const uint8_t *>(str), len);
}

void aurcor::MicroPython::mp_hal_stdout_tx_strn(const uint8_t *str, size_t len) {
	logger_.crit(F("[%s] Write to stdout with no implementation"), name_.c_str());
	abort();
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

void aurcor::MicroPythonFile::mp_hal_stdout_tx_strn(const uint8_t *str, size_t len) {
	stdout_.print(reinterpret_cast<const char *>(str), len);
}
