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

#pragma once

#include <Arduino.h>

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <uuid/log.h>

namespace aurcor {

using HeapData = std::unique_ptr<uint8_t, decltype(::free)*>;

class Heaps;

class Heap {
public:
	Heap(std::shared_ptr<Heaps> heaps, HeapData data, size_t size);
	~Heap();

	uint8_t *begin() { return data_.get(); }
	uint8_t *end() { return data_.get() + size_; }
	size_t size() const { return size_; }

private:
	std::weak_ptr<Heaps> heaps_;
	HeapData data_;
	const size_t size_;
};

class Heaps: public std::enable_shared_from_this<Heaps> {
	friend Heap;

public:
	Heaps(size_t size, uint32_t caps, size_t count = 0);
	~Heaps() = default;

	bool resize(size_t count);
	std::unique_ptr<Heap> allocate();

private:
	static uuid::log::Logger logger_;

	void restore(HeapData data);

	const size_t size_;
	const uint32_t caps_;
	std::mutex mutex_;
	std::vector<HeapData> heaps_;
	size_t used_ = 0;
};

} // namespace aurcor
