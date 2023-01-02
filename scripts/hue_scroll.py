# aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
# Copyright 2023  Simon Arlott
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import aurcor
import math

aurcor.register_config({
	"repeat": ("float", 1.0),
	"duration": ("s32", 21000),
})
config = {}

def generate(config):
	length = aurcor.length()
	step = config["repeat"] / aurcor.length()
	hue = (aurcor.next_time_ms() % config["duration"]) / config["duration"]
	while True:
		yield hue
		hue += step

while True:
	if aurcor.config(config):
		if not math.isfinite(config["repeat"]):
			config["repeat"] = 1.0
		config["duration"] = max(2, config["duration"])
	aurcor.output_exp_hsv(generate(config))
