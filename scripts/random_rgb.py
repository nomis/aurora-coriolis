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
	"colours": ("set_rgb", [0]),
	"duration": ("s32", 10000),
})

def generate(without=None):
	if without is None or len(colours) < 2:
		return random.choice(colours)
	else:
		return random.choice(list(config["colours"] - set((without,))))

def fill():
	global buffer

	buffer = [0] * aurcor.length()
	for pos in range(0, len(buffer)):
		buffer[pos] = generate()

def replace(count):
	if count == 1:
		pos = random.randint(0, len(buffer) - 1)
		buffer[pos] = generate(buffer[pos])
	else:
		positions = set(range(0, len(buffer)))
		while count > 0:
			pos = random.choice(list(positions))
			buffer[pos] = generate(buffer[pos])
			positions -= set((pos,))
			count -= 1

def change():
	global last

	now = aurcor.ticks64_us()
	if now - last >= interval:
		replace(min((now - last) // interval, len(buffer)))
		last = now

config = {}
last = aurcor.ticks64_us()

while True:
	if aurcor.config(config):
		colours = list(config["colours"])
		interval = max(1, config["duration"] * 1000 // aurcor.length())
		fill()

	change()
	aurcor.output_rgb(buffer)
