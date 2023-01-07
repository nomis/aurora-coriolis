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
import time

# 1x speed = 1125ms duration for 50 LEDs
INTERVAL_US = const(12500)

def create_config(enabled=False):
	return {
		"sweep.enabled": ("bool", enabled),
		"sweep.active_length": ("float", 12), # Percent
		"sweep.fade_length": ("float", 8), # Percent
		"sweep.fade_rate1": ("float", 0.5),
		"sweep.fade_rateN": ("float", 0.75),
		"sweep.speed": ("float", 1), # Relative speed based on length
		"sweep.duration": ("s32", None), # Duration in ms; overrides speed
	}

def apply_default_config(default_config={}):
	default_config = default_config.copy()

	if is_enabled:
		if "fps" in default_config:
			del default_config["fps"]

		if "wait_ms" in default_config:
			del default_config["wait_ms"]

		default_config["wait_us"] = 0

	return default_config

def config_changed(config):
	global is_enabled, length, fade_multipliers, last_update_us, current_pos
	global active_length, fade_length, total_length, sweep_length, interval_us

	is_enabled = config["sweep.enabled"]

	if not is_enabled:
		return

	config["sweep.active_length"] = max(0.0, min(100.0, config["sweep.active_length"]))
	config["sweep.fade_length"] = max(0.0, min(max(0.0, 100.0 - config["sweep.active_length"]), config["sweep.fade_length"]))

	length = aurcor.length()
	active_length = max(1, round(length * config["sweep.active_length"] // 200))
	fade_length = max(0, round(length * config["sweep.fade_length"] // 200))
	total_length = active_length + fade_length
	sweep_length = max(2, length - 2 * active_length) + 1

	fade_multipliers = list([config["sweep.fade_rate1"] * (config["sweep.fade_rateN"] ** n) / aurcor.MAX_VALUE for n in range(0, fade_length)])

	current_pos = 0
	last_update_us = None

	if config["sweep.duration"] is None:
		interval_us = INTERVAL_US // max(1e-10, config["sweep.speed"])
	else:
		interval_us = config["sweep.duration"] * 1000 / (2 * sweep_length)
	interval_us = max(1, min(1000000, round(interval_us)))

def enabled():
	return is_enabled

def refresh():
	global current_pos, last_update_us

	now_us = aurcor.ticks64_us()

	if last_update_us is None:
		last_update_us = now_us
		return True

	elapsed_us = now_us - last_update_us
	if elapsed_us >= interval_us:
		current_pos = (current_pos + 1) % (2 * sweep_length)
		last_update_us = now_us
		return True

	return False

def sleep(max_us=1000000):
	now_us = aurcor.ticks64_us()
	timeout_us = min(interval_us, max_us)

	elapsed_us = now_us - last_update_us
	if elapsed_us < timeout_us:
		time.sleep_us(timeout_us - elapsed_us)

def apply_mask_hsv(values):
	if not is_enabled:
		return values

	if current_pos < sweep_length:
		pos = current_pos
	else:
		pos = sweep_length - (current_pos - sweep_length)

	pos = active_length + max(0, min(sweep_length - 1, pos)) - 1
	pos = max(active_length, min(length - active_length, pos))

	values = values.copy()
	for i in range(max(0, pos - total_length), min(length, pos - active_length)):
		hue, saturation, value = values[i]
		values[i] = hue, saturation, value * fade_multipliers[(pos - active_length) - i - 1]

	for i in range(pos + active_length + 1, min(length, pos + total_length + 1)):
		hue, saturation, value = values[i]
		values[i] = hue, saturation, value * fade_multipliers[i - (pos + active_length) - 1]

	for i in range(0, min(length, pos - total_length)):
		values[i] = (0, 0, 0)

	for i in range(pos + total_length + 1, length):
		values[i] = (0, 0, 0)

	return values

if __name__ == "__main__":
	import logging

	logging.warning("sweep is a library and should not be used directly")

	aurcor.output_defaults(fps=1)
	while True:
		aurcor.output_rgb([])
