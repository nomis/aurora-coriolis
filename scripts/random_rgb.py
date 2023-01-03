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
import random

import sweep

FROM_SET = const(0)
AUTO_TYPE1 = const(1)
AUTO_TYPE2 = const(2)
MAX_AUTO = const(2)

config = {
	"auto": ("s32", 0),
	"colours": ("set_rgb", [0]),
	"count": ("s32", 1),
	"duration": ("s32", 10000),
}
config.update(sweep.create_config())
aurcor.register_config(config)

def generate_from_set(without=None):
	if without is None or len(colours) < 2:
		return random.choice(colours)
	else:
		return random.choice(list(config["colours"] - set((without,))))

def generate_type1(without=None):
	return aurcor.exp_hsv_to_rgb_int(random.randint(0, aurcor.EXP_HUE_RANGE - 1), random.uniform(0.5, 1.0), random.uniform(0.5, 1.0))

def generate_type2(without=None):
	return aurcor.exp_hsv_to_rgb_int(random.randint(0, aurcor.EXP_HUE_RANGE - 1))

def fill():
	global buffer, positions

	buffer = [0] * aurcor.length()
	positions = list(range(0, aurcor.length()))
	for pos in positions:
		buffer[pos] = generate()

def shuffle(count):
	length = len(positions) - 1
	for pos in range(0, count):
		other = random.randint(pos, length)
		tmp = positions[pos]
		positions[pos] = positions[other]
		positions[other] = tmp

def replace(count):
	if count == 1:
		pos = random.randint(0, len(buffer) - 1)
		buffer[pos] = generate(buffer[pos])
	elif count >= len(buffer):
		for pos in positions:
			buffer[pos] = generate(buffer[pos])
	else:
		shuffle(count)
		for pos in range(0, count):
			buffer[positions[pos]] = generate(buffer[pos])

def change():
	global last

	now = aurcor.ticks64_us()
	if now - last >= interval:
		replace(min((now - last) // interval * config["count"], len(buffer)))
		last = now

last = aurcor.ticks64_us()

while True:
	if aurcor.config(config):
		colours = list(config["colours"])

		config["auto"] = max(0, min(MAX_AUTO, config["auto"]))
		if config["count"] == 0 or config["count"] > aurcor.length():
			config["count"] = aurcor.length()

		if config["auto"] == FROM_SET:
			generate = generate_from_set
		elif config["auto"] == AUTO_TYPE1:
			generate = generate_type1
			aurcor.output_defaults(profile=aurcor.profiles.HDR)
		elif config["auto"] == AUTO_TYPE2:
			generate = generate_type2
			aurcor.output_defaults(profile=aurcor.profiles.NORMAL)

		interval = max(1, config["count"] * config["duration"] * 1000 // aurcor.length())
		sweep.config_changed(config)
		fill()

	change()
	aurcor.output_rgb(sweep.apply_mask_rgb(config, buffer))
