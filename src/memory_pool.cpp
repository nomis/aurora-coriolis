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

#include "aurcor/memory_pool.h"

#include <Arduino.h>

#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include <uuid/log.h>

#include "app/gcc.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "mpool";

namespace aurcor {

uuid::log::Logger MemoryPool::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::KERN};

MemoryPool::MemoryPool(size_t size, uint32_t caps, size_t count)
		: size_(size), caps_(caps) {
	if (count > 0)
		resize(count);
}

bool MemoryPool::resize(size_t count) {
	std::lock_guard lock{mutex_};

	while (capacity_ < count) {
		MemoryAllocation data{reinterpret_cast<uint8_t*>(::heap_caps_malloc(size_, caps_)), ::free};

		if (!data) {
			logger_.emerg(F("Unable to allocate block with size %u caps 0x%08x (%u of %u)"),
				size_, caps_, capacity_ + 1, count);
			return false;
		}

		blocks_.push_back(std::move(data));
		capacity_++;
	}

	while (capacity_ > count && !blocks_.empty()) {
		blocks_.pop_back();
		capacity_--;
	}

	size_t actual = blocks_.size() + used_;

	logger_.trace(F("Allocated %u block%S with size %u caps 0x%08x"),
		actual, actual != 1 ? F("s") : F(""), size_, caps_);

	return true;
}

std::unique_ptr<MemoryBlock> MemoryPool::allocate(bool zero) {
	std::lock_guard lock{mutex_};

	if (!blocks_.empty()) {
		auto self = shared_from_this();
		auto block = std::make_unique<MemoryBlock>(self, std::move(blocks_.back()), size_);

		blocks_.pop_back();
		used_++;

		if (zero)
			std::memset(block->begin(), 0, block->size());
		return block;
	}

	return {};
}

void MemoryPool::restore(MemoryAllocation data) {
	std::lock_guard lock{mutex_};

	if (blocks_.size() + used_ <= capacity_)
		blocks_.push_back(std::move(data));

	used_--;
}

MemoryBlock::MemoryBlock(std::shared_ptr<MemoryPool> blocks, MemoryAllocation data, size_t size)
		: blocks_(blocks), data_(std::move(data)), size_(size) {
}

MemoryBlock::~MemoryBlock() {
	auto blocks = blocks_.lock();

	if (blocks)
		blocks->restore(std::move(data_));
}

} // namespace aurcor
