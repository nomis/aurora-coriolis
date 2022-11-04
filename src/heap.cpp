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

#include "aurcor/heap.h"

#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include <uuid/log.h>

#include "app/gcc.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "heap";

namespace aurcor {

uuid::log::Logger Heaps::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::KERN};

Heaps::Heaps(size_t size, uint32_t caps, size_t count)
		: size_(size), caps_(caps) {
	resize(count);
}

bool Heaps::resize(size_t count) {
	std::lock_guard<std::mutex> lock{mutex_};

	while (heaps_.size() + used_ < count) {
		HeapData data{reinterpret_cast<uint8_t*>(::heap_caps_malloc(size_, caps_)), ::free};

		if (!data) {
			logger_.emerg(F("Unable to allocate heap with size %u caps 0x%08x (%u of %u)"),
				size_, caps_, heaps_.size() + used_ + 1, count);
			return false;
		} else {
			logger_.trace(F("Allocated heap with size %u caps 0x%08x (%u of %u)"),
				size_, caps_, heaps_.size() + used_ + 1, count);
		}

		heaps_.push_back(std::move(data));
	}

	while (heaps_.size() + used_ > count && !heaps_.empty()) {
		heaps_.pop_back();
	}

	return true;
}

std::unique_ptr<Heap> Heaps::allocate() {
	std::lock_guard<std::mutex> lock{mutex_};

	if (!heaps_.empty()) {
		auto self = shared_from_this();
		auto heap = std::make_unique<Heap>(self, std::move(heaps_.back()), size_);

		heaps_.pop_back();
		used_++;

		::memset(heap->begin(), 0, heap->size());
		return heap;
	}

	return {};
}

void Heaps::restore(HeapData data) {
	std::lock_guard<std::mutex> lock{mutex_};

	heaps_.push_back(std::move(data));
	used_--;
}

Heap::Heap(std::shared_ptr<Heaps> heaps, HeapData data, size_t size)
		: heaps_(heaps), data_(std::move(data)), size_(size) {
}

Heap::~Heap() {
	auto heaps = heaps_.lock();

	if (heaps)
		heaps->restore(std::move(data_));
}

} // namespace aurcor
