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

aurcor.register_config({
	"type": ("s32", 1),
	"count": ("s32", 1),
	"duration": ("s32", 10000),
})

def generate():
	if config["type"] == 1:
		return aurcor.exp_hsv_to_rgb_int(random.randint(0, aurcor.EXP_HUE_RANGE - 1), random.uniform(0.5, 1.0), random.uniform(0.5, 1.0))
	elif config["type"] == 2:
		return aurcor.exp_hsv_to_rgb_int(random.randint(0, aurcor.EXP_HUE_RANGE - 1))

def fill():
	global buffer

	buffer = [0] * aurcor.length()
	for pos in range(0, len(buffer)):
		buffer[pos] = generate()

def replace(count):
	if count == 1:
		pos = random.randint(0, len(buffer) - 1)
		buffer[pos] = generate()
	elif count >= len(buffer):
		for pos in range(0, len(buffer)):
			buffer[pos] = generate()
	else:
		positions = set(range(0, len(buffer)))
		while count > 0:
			pos = random.choice(list(positions))
			buffer[pos] = generate()
			positions -= set((pos,))
			count -= 1

def change():
	global last

	now = aurcor.ticks64_us()
	if now - last >= interval:
		replace(min((now - last) // interval * config["count"], len(buffer)))
		last = now

config = {}
last = aurcor.ticks64_us()

while True:
	if aurcor.config(config):
		config["type"] = max(1, min(2, config["type"]))
		if config["count"] == 0 or config["count"] > aurcor.length():
			config["count"] = aurcor.length()

		if config["type"] == 1:
			aurcor.output_defaults(profile=aurcor.profiles.HDR)
		elif config["type"] == 2:
			aurcor.output_defaults(profile=aurcor.profiles.NORMAL)

		interval = max(1, config["count"] * config["duration"] * 1000 // aurcor.length())
		fill()

	change()
	aurcor.output_rgb(buffer)
