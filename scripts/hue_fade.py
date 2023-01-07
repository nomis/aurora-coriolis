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

import sweep

config = {
	"duration": ("s32", 25000),
	"real_time": ("bool", False),
}
config.update(sweep.create_config())
aurcor.register_config(config)

last_hue = None
while True:
	if aurcor.config(config):
		config["duration"] = max(2, config["duration"])

		interval_us = max(1, round((config["duration"] * 1000) / aurcor.EXP_HUE_RANGE))

		sweep.config_changed(config)
		aurcor.output_defaults(**sweep.apply_default_config())

	if config["real_time"]:
		next_output_ms = aurcor.next_time_ms()
	else:
		next_output_ms = aurcor.next_ticks64_ms()

	hue = aurcor.EXP_HUE_RANGE * (next_output_ms % config["duration"]) // config["duration"]

	if sweep.enabled():
		if sweep.refresh() or hue != last_hue:
			aurcor.output_exp_hsv(sweep.apply_mask_hsv([[hue, aurcor.MAX_SATURATION, aurcor.MAX_VALUE]] * aurcor.length()))
		else:
			sweep.sleep(interval_us)
	else:
		aurcor.output_exp_hsv([hue], repeat=True)

	last_hue = hue
