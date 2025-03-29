# aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
# Copyright 2024  Simon Arlott
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
import collections
import logging

config = {
	"profile": ("profile", aurcor.profiles.NORMAL),
}
aurcor.register_config(config)

buffer_len = aurcor.MAX_LEDS * 3
buffer = bytearray(buffer_len)

sources = collections.OrderedDict()
frames = 0
last_report_ms = aurcor.ticks64_ms()

def parse_warls(data, offset, length):
	while offset + 4 < length:
		i = data[offset] * 3
		if i < buffer_len:
			buffer[i] = data[offset + 1]
			buffer[i + 1] = data[offset + 2]
			buffer[i + 2] = data[offset + 3]
		offset += 4

def parse_drgb(data, offset, length):
	i = 0
	while offset + 3 < length and i < buffer_len:
		buffer[i] = data[offset] * 3
		buffer[i + 1] = data[offset + 1]
		buffer[i + 2] = data[offset + 2]
		i += 3
		offset += 3

def parse_drgbw(data, offset, length):
	i = 0
	while offset + 4 < length and i < buffer_len:
		buffer[i] = data[offset]
		buffer[i + 1] = data[offset + 1]
		buffer[i + 2] = data[offset + 2]
		i += 3
		offset += 4

def parse_dnrgb(data, offset, length):
	if offset + 2 > length:
		return

	i = ((data[offset] << 8) | data[offset + 1]) * 3
	offset += 2

	while offset + 3 < length and i < buffer_len:
		buffer[i] = data[offset]
		buffer[i + 1] = data[offset + 1]
		buffer[i + 2] = data[offset + 2]
		i += 3
		offset += 3

while True:
	if aurcor.config(config):
		aurcor.output_defaults(profile=config["profile"], wait_us=0)
		aurcor.output_rgb(buffer)

	for packet in aurcor.udp_receive():
		sources[packet.source_address] = sources.get(packet.source_address, 0) + 1
		frames += 1
		length = len(packet.data)

		if length < 2:
			continue

		# https://github.com/Aircoookie/WLED/wiki/UDP-Realtime-Control#udp-realtime
		if packet.data[0] == 1:
			parse_warls(packet.data, 2, length)
		elif packet.data[0] == 2:
			parse_drgb(packet.data, 2, length)
		elif packet.data[0] == 3:
			parse_drgbw(packet.data, 2, length)
		elif packet.data[0] == 4:
			parse_dnrgb(packet.data, 2, length)

	aurcor.output_rgb(buffer)

	now_ms = aurcor.ticks64_ms()
	if now_ms - last_report_ms >= 60000:
		logging.debug(f"Sources: {sources}, {frames / (now_ms - last_report_ms) * 1000} fps")

		sources = collections.OrderedDict()
		frames = 0
		last_report_ms = now_ms
