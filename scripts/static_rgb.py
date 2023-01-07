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

import sweep

config = {"colours": ("list_rgb", [0])}
config.update(sweep.create_config())
aurcor.register_config(config)

def repeat_colours():
	length = aurcor.length()

	remaining = length - len(config["colours"])
	while remaining > 0:
		config["colours"].extend(config["colours"][0:min(len(config["colours"]), remaining)])
		remaining = length - len(config["colours"])

while True:
	if aurcor.config(config):
		sweep.config_changed(config)

		if sweep.enabled():
			repeat_colours()
			colours = list(map(aurcor.rgb_to_hsv_tuple, config["colours"]))

		aurcor.output_defaults(**sweep.apply_default_config())

	if sweep.enabled():
		if sweep.refresh():
			aurcor.output_hsv(sweep.apply_mask_hsv(colours))
		else:
			sweep.sleep()
	else:
		aurcor.output_rgb(config["colours"], repeat=True)
