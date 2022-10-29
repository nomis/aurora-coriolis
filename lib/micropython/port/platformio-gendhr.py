#!/usr/bin/env python3
# aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
# Copyright 2022  Simon Arlott

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# PlatformIO usage:
#
# [env:...]
# extra_scripts = pre:lib/micropython/port/platformio-gendhr.py

import subprocess

run = False

def genhdr_before_build(env, node):
	global run

	if not run:
		subprocess.run(
			["make", "V=0", "-C", env["PROJECT_DIR"] + "/lib/micropython/port"],
			check=True, universal_newlines=True)
		run = True

	return node

if __name__ == "SCons.Script":
	Import("env")

	env.AddBuildMiddleware(genhdr_before_build, env["PROJECT_DIR"] + "/lib/micropython/*")
	env.AddBuildMiddleware(genhdr_before_build, env["PROJECT_INCLUDE_DIR"] + "/*")
	env.AddBuildMiddleware(genhdr_before_build, env["PROJECT_SRC_DIR"] + "/*")
