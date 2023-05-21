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
#
# https://github.com/FastLED/FastLED/commits/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
# Simple one-dimensional fire animation
#
# Copyright 2013 FastLED
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Fire2012 with programmable Color Palette
#
# This code is the same fire simulation as the original "Fire2012",
# but each heat cell's temperature is translated to color through a FastLED
# programmable color palette, instead of through the "HeatColor(...)" function.

# Fire2012 by Mark Kriegsman, July 2012
# as part of "Five Elements" shown here: https://youtu.be/knWiGsmgycY
#
# This basic one-dimensional 'fire' simulation works roughly as follows:
# There's a underlying array of 'heat' cells, that model the temperature
# at each point along the line.  Every cycle through the simulation,
# four steps are performed:
#  1) All cells cool down a little bit, losing heat to the air
#  2) The heat from each cell drifts 'up' and diffuses a little
#  3) Sometimes randomly new 'sparks' of heat are added at the bottom
#  4) The heat from each cell is rendered as a color into the leds array
#     The heat-to-color mapping uses a black-body radiation approximation.
#
# Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
#
# This simulation scales it self a bit depending on NUM_LEDS; it should look
# "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
#
# I recommend running this simulation at anywhere from 30-100 frames per second,
# meaning an interframe delay of about 10-35 milliseconds.
#
# Looks best on a high-density LED setup (60+ pixels/meter).
#
#
# There are two main parameters you can play with to control the look and
# feel of your fire: COOLING (used in step 1 above), and SPARKING (used
# in step 3 above).
#
# COOLING: How much does the air cool as it rises?
# Less cooling = taller flames.  More cooling = shorter flames.
#
# SPARKING: What chance (out of 1.0) is there that a new spark will be lit?
# Higher chance = more roaring fire.  Lower chance = more flickery fire.

import aurcor
import urandom

from gradient import GradientFromRGBList

FROM_LIST = const(0)
AUTO_EXP_HUE_FADE = const(1)
MAX_AUTO = const(2)

config = {
	"auto": ("s32", 0),
	"colours": ("list_rgb", [0]),
	"fps": ("s32", 60),
	"cooling": ("s32", 55), # 20 to 100
	"sparking": ("float", 0.47), # 0.19 to 0.78
	"hue_duration": ("s32", 25000),
	"real_time": ("bool", False),
}
aurcor.register_config(config)

def qadd8(a, b):
	return min(255, a + b)

def qsub8(a, b):
	return max(0, a - b)

length = 0

while True:
	if aurcor.config(config):
		aurcor.output_defaults(fps=config["fps"])

		palette = GradientFromRGBList(256, config["colours"])
		cooling = max(0, min(255, config["cooling"]))
		sparking = config["sparking"]

		if config["auto"] < 0 or config["auto"] > MAX_AUTO:
			config["auto"] = 0

		if length != aurcor.length():
			length = aurcor.length()
			buffer = [(0, 0, 0)] * length

			# Array of temperature readings at each simulation cell
			heat = [0] * length

	# Step 1.  Cool down every cell a little
	for i in range(0, length):
		heat[i] = qsub8(heat[i], urandom.randint(0, ((cooling * 10) // length) + 2))

	# Step 2.  Heat from each cell drifts 'up' and diffuses a little
	for k in range(length - 1, 1, -1):
		heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) // 3

	# Step 3.  Randomly ignite new 'sparks' of heat near the bottom
	if urandom.random() < sparking:
		y = urandom.randint(0, min(7, length))
		heat[y] = qadd8(heat[y], urandom.randint(160, 255))

	# Step 4.  Map from heat cells to LED colours
	if config["auto"] == FROM_LIST:
		for j in range(0, length):
			# Scale the heat value from 0-255 down to 0-240
			# for best results with colour palettes
			buffer[j] = palette.rgb(heat[j] * 240 // 255)

		aurcor.output_rgb(buffer)
	elif config["auto"] == AUTO_EXP_HUE_FADE:
		if config["real_time"]:
			next_output_ms = aurcor.next_time_ms()
		else:
			next_output_ms = aurcor.next_ticks64_ms()

		hue = aurcor.EXP_HUE_RANGE * (next_output_ms % config["hue_duration"]) // config["hue_duration"]

		for j in range(0, length):
			h = heat[j] * 240 // 255
			if h < 64: # black to pure hue at three-quarters brightness
				buffer[j] = hue, aurcor.MAX_SATURATION, h / 64 * 0.75
			elif h < 128: # pure hue at three-quarters brightness to half "whitened" hue at full brightness
				buffer[j] = hue, 1.0 - (h & 0x3F) / 64 * 0.5, 0.75 + (h & 0x3F) / 64 * 0.25
			elif h < 192: # half "whitened" hue at full brightness to white
				buffer[j] = hue, 0.5 + (h & 0x3F) / 64 * 0.5, aurcor.MAX_VALUE
			else: # white to black
				buffer[j] = 0, 0, 1.0 - (h & 0x3F) / 64

		hue += 1
		aurcor.output_exp_hsv(buffer)
