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

#include "aurcor/micropython.h"

#include <memory>
#include <string>

#include <uuid/log.h>

class TestMicroPython: public aurcor::MicroPython {
public:
	static void init();
	static void tearDown();

	TestMicroPython(std::shared_ptr<aurcor::LEDBus> bus);
	virtual ~TestMicroPython() = default;

	void run(std::string script);

	std::string output_;
	int ret_{-1};
	bool force_exit_{false};
	bool stop_failed_{false};

protected:
	void main() override;

	void mp_hal_stdout_tx_strn(const uint8_t *str, size_t len) override;

	uuid::log::Level modulogging_effective_level() override;
	std::unique_ptr<aurcor::micropython::Print> modulogging_print(uuid::log::Level level) override;

private:
	std::string script_;
};
