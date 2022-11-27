/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022  Simon Arlott
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unity.h>

#include "app/fs.h"
#include "aurcor/led_profiles.h"

#include "test_micropython.h"

using aurcor::LEDProfile;
using aurcor::LEDProfiles;

static void test_save() {
	LEDProfiles profiles{"test_save"};
	auto &profile = profiles.get(LED_PROFILE_NORMAL);

	profile.clear();
	TEST_ASSERT_EQUAL_INT(1, profile.indexes().size());
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.set(50, 101, 102, 103));
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.set(100, 151, 152, 153));
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.set(150, 201, 202, 203));
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.set(200, 251, 252, 253));
	TEST_ASSERT_EQUAL_INT(5, profile.indexes().size());

	uint8_t r, g, b;
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.get(0, r, g, b));
	TEST_ASSERT_EQUAL_INT(8, r);
	TEST_ASSERT_EQUAL_INT(8, g);
	TEST_ASSERT_EQUAL_INT(8, b);

	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.get(50, r, g, b));
	TEST_ASSERT_EQUAL_INT(101, r);
	TEST_ASSERT_EQUAL_INT(102, g);
	TEST_ASSERT_EQUAL_INT(103, b);

	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.get(100, r, g, b));
	TEST_ASSERT_EQUAL_INT(151, r);
	TEST_ASSERT_EQUAL_INT(152, g);
	TEST_ASSERT_EQUAL_INT(153, b);

	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.get(150, r, g, b));
	TEST_ASSERT_EQUAL_INT(201, r);
	TEST_ASSERT_EQUAL_INT(202, g);
	TEST_ASSERT_EQUAL_INT(203, b);

	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.get(200, r, g, b));
	TEST_ASSERT_EQUAL_INT(251, r);
	TEST_ASSERT_EQUAL_INT(252, g);
	TEST_ASSERT_EQUAL_INT(253, b);

	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profiles.save(LED_PROFILE_NORMAL));

	auto file = app::FS.open("/profiles/test_save.normal.cbor");
	TEST_ASSERT_TRUE(file);
	if (file) {
		std::vector<uint8_t> data(file.size());
		TEST_ASSERT_EQUAL_INT(data.size(), file.read(data.data(), data.size()));

		TEST_ASSERT_EQUAL_INT(50, data.size());
		TEST_ASSERT_EQUAL_HEX8_ARRAY(
			"\xd9\xd9\xf7" /* CBOR */
				"\x85" /* Array of 5 elements */
					"\x82" /* Array of 2 elements */
						"\x00" /* Int: 0 */
						"\x83" /* Array of 3 elements */
							"\x08" /* Int: 8 */
							"\x08" /* Int: 8 */
							"\x08" /* Int: 8 */
					"\x82" /* Array of 2 elements */
						"\x18\x32" /* Int: 50 */
						"\x83" /* Array of 3 elements */
							"\x18\x65" /* Int: 101 */
							"\x18\x66" /* Int: 102 */
							"\x18\x67" /* Int: 103 */
					"\x82" /* Array of 2 elements */
						"\x18\x64" /* Int: 100 */
						"\x83" /* Array of 3 elements */
							"\x18\x97" /* Int: 151 */
							"\x18\x98" /* Int: 152 */
							"\x18\x99" /* Int: 153 */
					"\x82" /* Array of 2 elements */
						"\x18\x96" /* Int: 150 */
						"\x83" /* Array of 3 elements */
							"\x18\xc9" /* Int: 201 */
							"\x18\xca" /* Int: 202 */
							"\x18\xcb" /* Int: 203 */
					"\x82" /* Array of 2 elements */
						"\x18\xc8" /* Int: 200 */
						"\x83" /* Array of 3 elements */
							"\x18\xfb" /* Int: 251 */
							"\x18\xfc" /* Int: 252 */
							"\x18\xfd" /* Int: 253 */,
			data.data(), 50);
	}
}

static void test_load() {
	LEDProfiles profiles{"test_load"};
	auto &profile = profiles.get(LED_PROFILE_NORMAL);

	profile.clear();
	TEST_ASSERT_EQUAL_INT(1, profile.indexes().size());
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.set(50, 101, 102, 103));
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.set(100, 151, 152, 153));
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.set(150, 201, 202, 203));
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.set(200, 251, 252, 253));
	TEST_ASSERT_EQUAL_INT(5, profile.indexes().size());
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profiles.save(LED_PROFILE_NORMAL));

	profile.clear();
	TEST_ASSERT_EQUAL_INT(1, profile.indexes().size());

	uint8_t r, g, b;
	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.get(50, r, g, b));
	TEST_ASSERT_EQUAL_INT(8, r);
	TEST_ASSERT_EQUAL_INT(8, g);
	TEST_ASSERT_EQUAL_INT(8, b);

	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profiles.load(LED_PROFILE_NORMAL));
	TEST_ASSERT_EQUAL_INT(5, profile.indexes().size());

	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.get(50, r, g, b));
	TEST_ASSERT_EQUAL_INT(101, r);
	TEST_ASSERT_EQUAL_INT(102, g);
	TEST_ASSERT_EQUAL_INT(103, b);

	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.get(100, r, g, b));
	TEST_ASSERT_EQUAL_INT(151, r);
	TEST_ASSERT_EQUAL_INT(152, g);
	TEST_ASSERT_EQUAL_INT(153, b);

	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.get(150, r, g, b));
	TEST_ASSERT_EQUAL_INT(201, r);
	TEST_ASSERT_EQUAL_INT(202, g);
	TEST_ASSERT_EQUAL_INT(203, b);

	TEST_ASSERT_EQUAL_INT(LEDProfile::Result::OK, profile.get(200, r, g, b));
	TEST_ASSERT_EQUAL_INT(251, r);
	TEST_ASSERT_EQUAL_INT(252, g);
	TEST_ASSERT_EQUAL_INT(253, b);
}

void tearDown(void) {
	TestMicroPython::tearDown();
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	TestMicroPython::init();

	RUN_TEST(test_save);
	RUN_TEST(test_load);

	return UNITY_END();
}
