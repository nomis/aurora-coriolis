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
import random

MODE_LOWER_VALUE = const(0) # Start at maximum level, twinkling to minimum
MODE_RAISE_VALUE = const(1) # Start at minimum level, twinkling to maximum
MODE_RAISE_SATURATION_VALUE = const(2) # Start saturation at zero, twinkling to full; value as above
MAX_MODE = const(2)

def create_config(enabled=False):
	return {
		"twinkle.enabled": ("bool", enabled),
		"twinkle.number": ("s32", 2), # Per 100 LEDs
		"twinkle.period": ("s32", 250), # Start "number" LEDs twinkling every "period" milliseconds
		"twinkle.duration": ("s32", 450), # Per LED twinkle duration in milliseconds
		"twinkle.level": ("float", 0.2), # Minimum level for brightness multiplier
		"twinkle.mode": ("s32", MODE_LOWER_VALUE),
	}

def config_changed(config):
	global is_enabled, length, active_count, max_count, positions, start_times
	global level_minimum, level_range, invert, inactive_value_multiplier
	global twinkle_mode, interval_us, duration_us, last

	is_enabled = config["twinkle.enabled"]

	if not is_enabled:
		return

	if config["twinkle.mode"] < 0 or config["twinkle.mode"] > MAX_MODE:
		config["twinkle.mode"] = 0

	twinkle_mode = config["twinkle.mode"]

	config["twinkle.number"] = max(0, min(100, config["twinkle.number"]))
	config["twinkle.level"] = max(0.0, min(1.0, config["twinkle.level"]))

	length = aurcor.length()
	active_count = 0
	max_count = max(1, min(length, round(length * config["twinkle.number"] / 100)))
	start_times = [None] * max_count

	# [0:active_count] are active
	# [active_count:] are inactive
	positions = list(range(0, length))

	level_minimum = config["twinkle.level"]
	level_range = (1.0 - level_minimum)
	inactive_value_multiplier = level_minimum / aurcor.MAX_VALUE

	interval_us = max(1, config["twinkle.period"] * 1000 // config["twinkle.number"])
	duration_us = max(2, config["twinkle.duration"] * 1000)
	last = aurcor.next_ticks64_us()

def enabled():
	return is_enabled

def start(now, count):
	global active_count

	for range in(0, count):
		if active_count >= max_count:
			return

		n = random.randint(active_count, length - 1)

		tmp = positions[active_count]
		positions[active_count] = positions[n]
		positions[n] = tmp

		start_times[active_count] = now
		active_count += 1

def stop(indexes):
	global active_count

	for n in indexes:
		assert active_count > 0
		active_count -= 1

		tmp = positions[active_count]
		positions[active_count] = positions[n]
		positions[n] = tmp

		tmp = start_times[active_count]
		start_times[active_count] = start_times[n]
		start_times[n] = tmp

def change(now):
	global last

	if now - last >= interval_us:
		if active_count < max_count:
			count = (now - last) // interval_us
			start(now, min(count, max_count - active_count))
			last += count * interval_us
		else:
			last = now

def apply_hsv(values):
	if not is_enabled:
		return values

	half_duration_us = duration_us // 2
	max_value = aurcor.MAX_VALUE
	values = values.copy()

	now = aurcor.next_ticks64_us()
	change(now)
	stopped = []

	if twinkle_mode == MODE_LOWER_VALUE:
		for n in range(0, active_count):
			elapsed_us = now - start_times[n]
			if elapsed_us >= duration_us:
				stopped.append(n)
				continue

			pos = positions[n]
			hue, saturation, value = values[pos]

			if elapsed_us < half_duration_us:
				value = value / max_value * (level_minimum + level_range * ((half_duration_us - elapsed_us - 1) / half_duration_us))
			else:
				value = value / max_value * (level_minimum + level_range * ((elapsed_us - half_duration_us) / half_duration_us))

			values[pos] = hue, saturation, value
	elif twinkle_mode == MODE_RAISE_VALUE:
		for n in range(0, active_count):
			elapsed_us = now - start_times[n]
			if elapsed_us >= duration_us:
				stopped.append(n)
				continue

			pos = positions[n]
			hue, saturation, value = values[pos]

			if elapsed_us < half_duration_us:
				value = value / max_value * (level_minimum + level_range * ((elapsed_us + 1) / half_duration_us))
			else:
				value = value / max_value * (level_minimum + level_range * ((duration_us - elapsed_us) / half_duration_us))

			values[pos] = hue, saturation, value
	elif twinkle_mode == MODE_RAISE_SATURATION_VALUE:
		for n in range(0, active_count):
			elapsed_us = now - start_times[n]
			if elapsed_us >= duration_us:
				stopped.append(n)
				continue

			pos = positions[n]
			hue, saturation, value = values[pos]

			if elapsed_us < half_duration_us:
				saturation = (elapsed_us + 1) / half_duration_us
			else:
				saturation = (duration_us - elapsed_us) / half_duration_us
			value = value / max_value * (level_minimum + level_range * saturation)

			values[pos] = hue, saturation, value

	stop(stopped)

	if twinkle_mode == MODE_RAISE_VALUE:
		for pos in positions[active_count:]:
			hue, saturation, value = values[pos]
			values[pos] = hue, saturation, value * inactive_value_multiplier
	elif twinkle_mode == MODE_RAISE_SATURATION_VALUE:
		for pos in positions[active_count:]:
			hue, _, value = values[pos]
			values[pos] = hue, 0, value * inactive_value_multiplier

	return values

if __name__ == "__main__":
	import logging

	logging.warning("twinkle is a library and should not be used directly")

	aurcor.output_defaults(fps=1)
	while True:
		aurcor.output_rgb([])
