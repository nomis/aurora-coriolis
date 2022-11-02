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

#include <array>
#include <string>
#include <vector>

extern "C" {
	#include <py/mpprint.h>
}

#include <uuid/log.h>

namespace aurcor {

namespace micropython {

class Print {
public:
	Print() = default;
	virtual ~Print() = default;

	inline const mp_print_t *context() { return &context_; }
	virtual void print(const char *str, size_t len) = 0;

private:
	Print(Print&&) = delete;
	Print(const Print&) = delete;
	Print& operator=(Print&&) = delete;
	Print& operator=(const Print&) = delete;

	static void print_strn(void *env, const char *str, size_t len);

	const mp_print_t context_{this, &print_strn};
	std::vector<char> text_;
};

class LinePrint: public Print {
public:
	LinePrint() = default;
	virtual ~LinePrint() = default;

	void print(const char *str, size_t len) final override;

protected:
	virtual void begin_line();
	virtual void print_part_line(const char *str, size_t len) = 0;
	virtual void end_line() = 0;

	bool line_started() const { return line_; }

	bool line_{false};
};

class LineWrapPrint: public LinePrint {
public:
	LineWrapPrint(size_t line_length);
	virtual ~LineWrapPrint();

protected:
	void print_part_line(const char *str, size_t len) final override;
	void end_line() final override;

	virtual void print_wrapped_line(const char *text, bool continuation) = 0;

private:
	void flush();

	std::vector<char> text_;
	bool continuation_{false};
};

class LogPrint: public LineWrapPrint {
public:
	static constexpr size_t MAX_LINE_LENGTH = 100;
	static constexpr char NORMAL_LINE = '>';
	static constexpr char CONTINUATION_LINE = '|';

	LogPrint(uuid::log::Logger &logger, uuid::log::Level level,
		const std::string &prefix);
	virtual ~LogPrint() = default;

protected:
	void print_wrapped_line(const char *text, bool continuation) override;

private:
	uuid::log::Logger &logger_;
	const std::string prefix_;
	const uuid::log::Level level_;
};

class PlatformPrint: public LinePrint {
public:
	PlatformPrint(uuid::log::Level level);
	~PlatformPrint();

protected:
	void begin_line() override;
	void print_part_line(const char *str, size_t len) override;
	void end_line() override;

private:
	const std::array<char, 4> level_;
};

} // namespace micropython

} // namespace aurcor
