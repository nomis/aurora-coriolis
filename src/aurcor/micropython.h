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

#include <csetjmp>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>

extern "C" {
	#include <py/lexer.h>
	#include <py/mphal.h>
	#include <py/nlr.h>
}

#include <uuid/console.h>

#include "io_buffer.h"

namespace aurcor {

class MicroPython {
public:
	using run_function = std::function<void()>;

	static constexpr size_t HEAP_SIZE = 192 * 1024;

	MicroPython(run_function f);
	virtual ~MicroPython();

protected:
	static uuid::log::Logger logger_;

	void start();
	virtual void shutdown() = 0;
	void stop();

	inline bool heap_available() const { return heap_ != nullptr; }
	inline bool running() const { return running_; }

	friend int ::mp_hal_stdin_rx_chr(void);
	virtual int mp_hal_stdin_rx_chr(void) = 0;

	friend void ::mp_hal_stdout_tx_strn(const char *str, size_t len);
	virtual void mp_hal_stdout_tx_strn(const uint8_t *str, size_t len) = 0;

private:
	static __thread MicroPython *self_;

	void running_thread();

	friend void ::nlr_jump_fail(void *val);
	void nlr_jump_fail(void *val) __attribute__((noreturn));

	friend mp_lexer_t *::mp_lexer_new_from_file(const char *filename);
	mp_lexer_t *mp_lexer_new_from_file(const char *filename);

	uint8_t *heap_;
	run_function main_;
	std::thread thread_;
	bool started_ = false;
	volatile bool running_ = false;
	bool stopped_ = false;
	const __FlashStringHelper *where_;
	jmp_buf abort_;
};

class MicroPythonShell: public MicroPython,
		public std::enable_shared_from_this<MicroPythonShell> {

public:
	static constexpr size_t STDIN_LEN = 32;
	static constexpr size_t STDOUT_LEN = 128;

	MicroPythonShell();

	void start(uuid::console::Shell &shell);

protected:
	void shutdown() override;

	int mp_hal_stdin_rx_chr(void) override;
	void mp_hal_stdout_tx_strn(const uint8_t *str, size_t len) override;

private:
	bool shell_foreground(uuid::console::Shell &shell, bool stop);

	IOBuffer<uint_fast8_t,STDIN_LEN> stdin_;
	IOBuffer<uint_fast8_t,STDOUT_LEN> stdout_;
};

} // namespace aurcor
