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

# 1x speed = 5s duration for 1 burst on 200 LEDs
DURATION_PER_LED_US = const(18727)

config = {
	"colours": ("list_rgb", [0]),
	"number": ("s32", 3),
	"fade_rate1": ("float", 0.5),
	"fade_rateN": ("float", 0.75),
	"speed": ("float", 1), # Relative speed based on length
	"duration": ("s32", None), # Duration in ms; overrides speed
	"real_time": ("bool", False),
}
aurcor.register_config(config)

def generate():
	if config["duration"] == 0:
		yield from ()

	length = aurcor.length()

	if config["real_time"]:
		next_output_us = aurcor.next_time_us()
	else:
		next_output_us = aurcor.next_ticks64_us()

	burst_count = config["number"]
	colour_count = len(config["colours"])
	fade_rate1 = config["fade_rate1"]
	fade_rateN = config["fade_rateN"]

	current_us = (next_output_us % total_duration_us)
	burst_idx = current_us // burst_duration_us
	colour_idx = ((next_output_us // total_duration_us) * burst_count) % colour_count

	if burst_idx >= burst_count:
		current_pos = round(blank_length * (((current_us - total_burst_duration_us) % blank_duration_us) / blank_duration_us))
	else:
		current_pos = round(burst_length * ((current_us % burst_duration_us) / burst_duration_us))

	while True:
		if burst_idx >= burst_count:
			for pos in range(current_pos, blank_length):
				yield 0

			current_pos = 0
			burst_idx = 0
			colour_idx = (colour_idx + burst_count) % colour_count
		else:
			colour_rgb = config["colours"][(colour_idx + burst_idx) % colour_count]
			colour_hsv = aurcor.rgb_to_hsv_tuple(colour_rgb)

			for pos in range(current_pos, burst_length):
				if pos < fade_length:
					hue, saturation, value = colour_hsv
					power = fade_length - pos - 1
					value = value / aurcor.MAX_VALUE * fade_rate1 * (fade_rateN ** power)
					yield aurcor.hsv_to_rgb_int(hue, saturation, value)
				elif pos < fade_active_length:
					yield colour_rgb
				elif pos < fade_active_fade_length:
					hue, saturation, value = colour_hsv
					power = fade_length - (fade_active_fade_length - pos) - 1
					value = value / aurcor.MAX_VALUE * fade_rate1 * (fade_rateN ** power)
					yield aurcor.hsv_to_rgb_int(hue, saturation, value)
				else:
					yield 0

			current_pos = 0
			burst_idx += 1

aurcor.output_defaults(reverse=True)

while True:
	if aurcor.config(config):
		if config["duration"] == 1:
			config["duration"] = 2

		config["number"] = max(1, min(aurcor.length(), config["number"]))

		length = aurcor.length()
		active_length = length // (2 + config["number"]) // 3
		fade_length = length // (2 + config["number"]) // 6
		fade_active_length = fade_length + active_length + 1 + active_length
		fade_active_fade_length = fade_active_length + fade_length
		burst_length = (fade_length + active_length + length) // config["number"]

		single_active_length = length // 9
		single_fade_length = length // 18
		blank_length = 1 + single_active_length + single_fade_length

		if config["duration"] is None:
			interval_us = max(1, round(DURATION_PER_LED_US / max(1e-10, config["speed"])))
		else:
			total_length = burst_length * config["number"] + blank_length
			interval_us = max(1, round(config["duration"] * 1000 / total_length))

		burst_duration_us = burst_length * interval_us
		blank_duration_us = blank_length * interval_us
		total_burst_duration_us = burst_duration_us * config["number"]
		total_duration_us = total_burst_duration_us + blank_duration_us

	aurcor.output_rgb(generate())
