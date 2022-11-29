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

#pragma once

#include <Arduino.h>

#include <atomic>
#include <csetjmp>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>

extern "C" {
	#include <py/builtin.h>
	#include <py/lexer.h>
	#include <py/mphal.h>
	#include <py/mpstate.h>
	#include <py/nlr.h>
	#include <shared/runtime/interrupt_char.h>
	#include <shared/runtime/pyexec.h>
}

#include <uuid/console.h>
#include <uuid/log.h>

#include "io_buffer.h"
#include "memory_pool.h"
#include "modaurcor.h"
#include "modulogging.h"
#include "mp_print.h"

namespace aurcor {

class LEDBus;

class MicroPython {
	friend aurcor::micropython::PyModule;
	friend aurcor::micropython::ULogging;

public:
	static constexpr size_t HEAP_SIZE = 192 * 1024;
	static constexpr size_t PYSTACK_SIZE = 4 * 1024;
	static constexpr size_t TASK_STACK_SIZE = 12 * 1024;
	static constexpr size_t TASK_STACK_MARGIN = 4 * 1024;
	static constexpr size_t TASK_EXC_STACK_MARGIN = 2 * 1024;
	static constexpr size_t TASK_STACK_LIMIT = TASK_STACK_SIZE - TASK_STACK_MARGIN;
	static constexpr size_t TASK_EXC_STACK_LIMIT = TASK_STACK_SIZE - TASK_EXC_STACK_MARGIN;
	static_assert(TASK_STACK_LIMIT < TASK_STACK_SIZE, "Task stack limit must be lower than task stack size");
	static_assert(TASK_STACK_LIMIT < TASK_EXC_STACK_LIMIT, "Task stack limit must be lower than task exception stack limit");

	static void setup(size_t pool_count);
	const std::string& name() const { return name_; }
	bool stop();

	virtual ~MicroPython();

protected:
	class AccessState {
	public:
		AccessState(MicroPython &mp);
		bool enable();
		void disable();
		~AccessState();

	private:
		MicroPython &mp_;
		std::lock_guard<std::mutex> state_lock_;
		std::unique_lock<std::mutex> atomic_section_;
		bool enabled_{false};
	};
	friend AccessState;

	static uuid::log::Logger logger_;

	MicroPython(const std::string &name, std::shared_ptr<LEDBus> bus);

	bool start();
	virtual void main();
	virtual void shutdown();

	inline bool memory_blocks_available() const { return heap_ && pystack_ && ledbufs_; }
	inline bool running() const { return running_; }

	virtual void state_copy();
	virtual void state_reset();

	void force_exit();
	void abort() __attribute__((noreturn));

	friend int ::mp_hal_stdin_rx_chr(void);
	virtual int mp_hal_stdin_rx_chr(void);

	friend void ::mp_hal_stdout_tx_strn(const char *str, size_t len);
	virtual void mp_hal_stdout_tx_strn(const uint8_t *str, size_t len);

	virtual uuid::log::Level modulogging_effective_level();
	virtual std::unique_ptr<aurcor::micropython::Print> modulogging_print(uuid::log::Level level);

	const std::string name_;

private:
	static inline MicroPython& current() { return *self_; }

	static thread_local MicroPython *self_;
	static std::shared_ptr<MemoryPool> heaps_;
	static std::shared_ptr<MemoryPool> pystacks_;
	static std::shared_ptr<MemoryPool> ledbufs_;

	void running_thread();

	friend void ::nlr_jump_fail(void *val);
	void nlr_jump_fail(void *val) __attribute__((noreturn));
	void log_exception(mp_obj_t exc, uuid::log::Level level);

	friend mp_uint_t ::mp_hal_begin_atomic_section(void);
	void mp_hal_begin_atomic_section();

	friend void ::mp_hal_end_atomic_section(void);
	void mp_hal_end_atomic_section();

	friend ::mp_lexer_t *::mp_lexer_new_from_file(const char *filename);
	::mp_lexer_t *mp_lexer_new_from_file(const char *filename);

	friend typeof(::mp_import_stat_t) ::mp_import_stat(const char *path);
	::mp_import_stat_t mp_import_stat(const char *path);

	std::unique_ptr<MemoryBlock> heap_;
	std::unique_ptr<MemoryBlock> pystack_;
	std::unique_ptr<MemoryBlock> ledbuf_;
	std::thread thread_;
	bool started_{false};
	std::mutex active_;
	std::atomic<bool> running_{false};
	bool stopped_{false};
	const char *where_;
	jmp_buf abort_;
	bool in_nlr_fail_{false};

	std::mutex state_mutex_;
#if MICROPY_INSTANCE_PER_THREAD
	typeof(mp_state_ctx_thread) state_ctx_{nullptr};
#endif
	typeof(pyexec_system_exit) *exec_system_exit_{nullptr};
	mp_obj_exception_t system_exit_exc_{};

	std::mutex atomic_section_mutex_;

	micropython::PyModule modaurcor_;
	micropython::ULogging modulogging_;
};

class MicroPythonShell: public MicroPython,
		public std::enable_shared_from_this<MicroPythonShell> {

public:
	static constexpr size_t STDIN_LEN = 32;
	static constexpr size_t STDOUT_LEN = 128;

	MicroPythonShell(const std::string &name, std::shared_ptr<LEDBus> bus);
	~MicroPythonShell() override;

	bool start(uuid::console::Shell &shell);
	bool shell_foreground(uuid::console::Shell &shell, bool stop);

protected:
	void main() override;
	void shutdown() override;

	void state_copy() override;
	void state_reset() override;

	int mp_hal_stdin_rx_chr(void) override;
	void mp_hal_stdout_tx_strn(const uint8_t *str, size_t len) override;

	uuid::log::Level modulogging_effective_level() override;
	std::unique_ptr<aurcor::micropython::Print> modulogging_print(uuid::log::Level level) override;

private:
	bool interrupt_char(int c);

	IOBuffer stdin_{STDIN_LEN};
	IOBuffer stdout_{STDOUT_LEN};

	typeof(mp_interrupt_char) *interrupt_char_{nullptr};
};

} // namespace aurcor
