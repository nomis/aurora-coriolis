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

import twinkle

config = {
	"repeat": ("float", 1),
	"duration": ("s32", 25000),
	"real_time": ("bool", False),
}
config.update(twinkle.create_config())
aurcor.register_config(config)

def generate():
	step = config["repeat"] / aurcor.length()

	if config["duration"] > 0:
		if config["real_time"]:
			next_output_ms = aurcor.next_time_ms()
		else:
			next_output_ms = aurcor.next_ticks64_ms()

		hue = (next_output_ms % config["duration"]) / config["duration"]
	else:
		hue = 0

	for n in range(0, aurcor.length()):
		yield [hue, aurcor.MAX_SATURATION, aurcor.MAX_VALUE]
		hue += step

config = {}

while True:
	if aurcor.config(config):
		if config["duration"] == 1:
			config["duration"] = 2

		twinkle.config_changed(config)

	if twinkle.enabled():
		aurcor.output_exp_hsv(twinkle.apply_hsv(list(generate())))
	else:
		aurcor.output_exp_hsv(list(generate()))
