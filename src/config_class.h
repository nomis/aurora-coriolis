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

#define MCU_APP_CONFIG_DATA \
	MCU_APP_CONFIG_SIMPLE(std::string, "", default_bus, "", "") \
	MCU_APP_CONFIG_SIMPLE(std::string, "", download_url, "", "")

public:
	std::string default_bus() const;
	void default_bus(const std::string &default_bus);

	std::string download_url() const;
	void download_url(const std::string &download_url);

private:
	static std::string default_bus_;
	static std::string download_url_;
