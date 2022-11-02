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

#include "aurcor/mp_print.h"

#include <algorithm>
#include <string>

extern "C" {
	#include <py/mphal.h>
}

#include <uuid/common.h>
#include <uuid/log.h>

namespace aurcor {

namespace micropython {

void Print::print(const char *str, size_t len) {
}

void Print::print_strn(void *env, const char *str, size_t len) {
	reinterpret_cast<Print*>(env)->print(str, len);
}

void LinePrint::begin_line() {
}

void LinePrint::print(const char *str, size_t len) {
	const char *pos = str;

	while (len-- > 0) {
		if (!line_) {
			line_ = true;
			begin_line();
		}

		if (*pos == '\r' || *pos == '\n') {
			if (line_) {
				if (pos - str > 0)
					print_part_line(str, pos - str);
				end_line();
				line_ = false;
			}

			str = pos + 1;

			if (len > 0 && *pos == '\r' && *str == '\n') {
				len--;
				pos++;
				str++;
			}
		}
		pos++;
	}

	if (pos - str > 0)
		print_part_line(str, pos - str);
}

LineWrapPrint::LineWrapPrint(size_t line_length) {
	text_.reserve(std::max((size_t)1, line_length) + 1);
}

LineWrapPrint::~LineWrapPrint() {
	if (line_started())
		end_line();
}

void LineWrapPrint::print_part_line(const char *str, size_t len) {
	while (len > 0) {
		size_t available = (text_.capacity() - 1) - text_.size();

		if (available > len)
			available = len;

		text_.insert(text_.end(), str, str + available);
		str += available;
		len -= available;

		if (text_.size() == text_.capacity() - 1) {
			flush();
			continuation_ = true;
		}
	}
}

void LineWrapPrint::end_line() {
	flush();
	continuation_ = false;
}

void LineWrapPrint::flush() {
	if (!text_.empty()) {
		text_.push_back('\0');
		print_wrapped_line(text_.data(), continuation_);
		text_.clear();
	}
}

LogPrint::LogPrint(uuid::log::Logger &logger, uuid::log::Level level,
		const std::string &prefix) : LineWrapPrint(MAX_LINE_LENGTH),
		logger_(logger), prefix_(prefix), level_(level) {
}

void LogPrint::print_wrapped_line(const char *text, bool continuation) {
	logger_.log(level_, logger_.facility(), F("%s%c %s"), prefix_.c_str(),
		continuation ? CONTINUATION_LINE : NORMAL_LINE, text);
}

PlatformPrint::PlatformPrint(uuid::log::Level level)
	: level_({' ', uuid::log::format_level_char(level), ':', ' '}) {
}

PlatformPrint::~PlatformPrint() {
	if (line_started())
		end_line();
}

void PlatformPrint::begin_line() {
	auto uptime = uuid::log::format_timestamp_ms(uuid::get_uptime_ms(), 3);
	::mp_hal_stdout_tx_strn(uptime.c_str(), uptime.length());
	::mp_hal_stdout_tx_strn(level_.data(), level_.size());
}

void PlatformPrint::print_part_line(const char *str, size_t len) {
	::mp_hal_stdout_tx_strn(str, len);
}

void PlatformPrint::end_line() {
	static const std::array<char, 2> crlf{'\r', '\n'};

	::mp_hal_stdout_tx_strn(crlf.data(), crlf.size());
}

} // namespace micropython

} // namespace aurcor
