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

#include "test_micropython.h"

#include <unity.h>
#include <Arduino.h>
#include <unistd.h>

extern "C" {
	#include <py/compile.h>
	#include <py/lexer.h>
	#include <py/nlr.h>
	#include <py/qstr.h>
}

#include <cstdio>
#include <memory>
#include <string>

#include <uuid/log.h>

#include "aurcor/micropython.h"
#include "test_mp_print.h"

using aurcor::MicroPython;

static uuid::log::PrintHandler logger{Serial};

void TestMicroPython::init() {
	uuid::log::Logger::register_handler(&logger, uuid::log::Level::ALL);
	MicroPython::setup(1);
}

void TestMicroPython::tearDown() {
	fflush(stderr);
	fprintf(stdout, "\n");
	fflush(stdout);
	logger.loop();
}

TestMicroPython::TestMicroPython(std::shared_ptr<aurcor::LEDBus> bus)
		: MicroPython("test", bus) {
}

void TestMicroPython::run(std::string script) {
	script_ = script;

	TEST_ASSERT_TRUE(MicroPython::start());

	alarm(10);
	while (running()) { delay(1); }
	alarm(10);
	while (!stop()) { delay(1); }
}

void TestMicroPython::main() {
	nlr_buf_t nlr;
	nlr.ret_val = NULL;
	if (nlr_push(&nlr) == 0) {
		mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, script_.c_str(), script_.length(), 0);
		mp_parse_tree_t parse_tree = mp_parse(lex, MP_PARSE_FILE_INPUT);
		mp_obj_t module_fun = mp_compile(&parse_tree, lex->source_name, false);

		mp_call_function_0(module_fun);
		mp_handle_pending(true);
		nlr_pop();

		ret_ = 0;
	} else {
		mp_handle_pending(false);
		mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));

		ret_ = 1;
	}
}

void TestMicroPython::mp_hal_stdout_tx_strn(const uint8_t *str, size_t len) {
	output_.append(reinterpret_cast<const char *>(str), len);
}

uuid::log::Level TestMicroPython::modulogging_effective_level() {
	return uuid::log::Level::ALL;
}

std::unique_ptr<aurcor::micropython::Print> TestMicroPython::modulogging_print(uuid::log::Level level) {
	return std::make_unique<TestPrint>(level);
}
