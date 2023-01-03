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

from micropython import const
import aurcor
import math

# 1x speed = 1500ms duration for 50 LEDs
DURATION_PER_LED_US = const(30 * 1000)

def create_config(enabled=False):
	return {
		"sweep.enabled": ("bool", enabled),
		"sweep.active_length": ("float", 12), # Percent
		"sweep.fade_length": ("float", 16), # Percent
		"sweep.fade_rate": ("float", 0.75),
		"sweep.speed": ("float", 1), # Relative speed based on length
		"sweep.duration": ("s32", None), # Duration in ms; overrides speed
		"sweep.real_time": ("bool", True),
	}

def config_changed(config):
	global active_length, fade_length, duration_us

	if not config["sweep.enabled"]:
		return

	config["sweep.active_length"] = max(0.0, min(100.0, config["sweep.active_length"]))
	config["sweep.fade_length"] = max(0.0, min(100.0, config["sweep.fade_length"]))

	if not math.isfinite(config["sweep.speed"]):
		config["sweep.speed"] = 1

	length = aurcor.length()
	active_length = max(1, length * config["sweep.active_length"] // 200)
	fade_length = max(0, length * config["sweep.fade_length"] // 100)

	if config["sweep.duration"] is None:
		duration_us = max(2, round(DURATION_PER_LED_US * length / max(1e-10, config["sweep.speed"])))
	else:
		duration_us = max(2, config["sweep.duration"] * 1000)

def enabled(config):
	return config["sweep.enabled"]

def apply_mask_rgb(config, values):
	if not enabled(config):
		return values

	return _apply_mask(config, values, True)

def apply_mask_hsv(config, values):
	if not enabled(config):
		return values

	return _apply_mask(config, values, False)

def _apply_mask(config, values, rgb):
	if config["sweep.real_time"]:
		next_output_us = aurcor.next_time_us()
	else:
		next_output_us = aurcor.next_ticks64_us()

	fade_rate = config["sweep.fade_rate"]
	total_length = active_length + fade_length

	length = max(2, aurcor.length() - 2 * active_length) + 1
	pos = round(((next_output_us % duration_us) / duration_us) * 2 * length)
	if pos >= length:
		pos = length - (pos - length)
	pos = active_length + max(0, min(length - 1, pos)) - 1
	pos = max(active_length, min(aurcor.length() - active_length, pos))

	for i in range(0, len(values)):
		if i >= pos - active_length and i <= pos + active_length:
			yield values[i]
		elif i >= pos - total_length and i <= pos + total_length:
			if rgb:
				hue, saturation, value = aurcor.rgb_to_hsv_tuple(values[i])
			elif type(values[i]) in (float, int):
				hue, saturation, value = values[i], aurcor.MAX_SATURATION, 1.0
			else:
				hue, saturation, value = values[i]

			if type(value) != float:
				value /= aurcor.MAX_VALUE

			if i < pos:
				power = (pos - active_length) - i
			else:
				power = i - (pos + active_length)

			value *= 0.5 * (fade_rate ** power)

			if rgb:
				yield aurcor.hsv_to_rgb_int(hue, saturation, value)
			else:
				yield hue, saturation, value
		elif rgb:
			yield 0
		else:
			yield 0, 0, 0


if __name__ == "__main__":
	import logging

	logging.warning("sweep is a library and should not be used directly")

	aurcor.output_defaults(fps=1)
	while True:
		aurcor.output_rgb([])
