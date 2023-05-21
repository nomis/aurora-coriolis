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

class Gradient:
	def __init__(self, length, palette):
		self.data = [0] * length
		n = 0

		for i in range(0, length):
			if n + 1 < len(palette) and i >= palette[n + 1][0]:
				n += 1

			a_idx, a_rgb = palette[n]
			b_idx, b_rgb = palette[(n + 1) % len(palette)]
			if b_idx < a_idx:
				b_idx = length

			if b_idx - a_idx > 0:
				b_scale = int(((i - a_idx) / (b_idx - a_idx)) * 255)
			else:
				b_scale = 0
			a_scale = 255 - b_scale

			red = ((a_rgb >> 16) & 0xFF) * a_scale // 255
			red += ((b_rgb >> 16) & 0xFF) * b_scale // 255
			green = ((a_rgb >> 8) & 0xFF) * a_scale // 255
			green += ((b_rgb >> 8) & 0xFF) * b_scale // 255
			blue = (a_rgb & 0xFF) * a_scale // 255
			blue += (b_rgb & 0xFF) * b_scale // 255

			self.data[i] = (red << 16) | (green << 8) | blue

	def rgb(self, index):
		return self.data[index]

class GradientFromRGBList(Gradient):
	def __init__(self, length, colours):
		palette = []
		n = 0

		for i in range(0, len(colours)):
			palette.append((n, colours[i]))
			n += length // len(colours)
			if n > length - 1:
				n = length - 1

		super().__init__(length, palette)

if __name__ == "__main__":
	import logging

	logging.warning("gradient is a library and should not be used directly")

	aurcor.output_defaults(fps=1)
	while True:
		aurcor.output_rgb([])
