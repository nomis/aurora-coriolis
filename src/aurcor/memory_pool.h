/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022-2023  Simon Arlott
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

class MemoryDeleter {
public:
	void operator()(uint8_t *data) { ::free(data); }
};

using MemoryAllocation = std::unique_ptr<uint8_t, MemoryDeleter>;

class MemoryPool;

class MemoryBlock {
public:
	MemoryBlock(std::shared_ptr<MemoryPool> blocks, MemoryAllocation data, size_t size);
	~MemoryBlock();

	uint8_t *begin() { return data_.get(); }
	uint8_t *end() { return data_.get() + size_; }
	size_t size() const { return size_; }

private:
	MemoryBlock(MemoryBlock&&) = delete;
	MemoryBlock(const MemoryBlock&) = delete;
	MemoryBlock& operator=(MemoryBlock&&) = delete;
	MemoryBlock& operator=(const MemoryBlock&) = delete;

	std::weak_ptr<MemoryPool> blocks_;
	MemoryAllocation data_;
	const size_t size_;
};

class MemoryPool: public std::enable_shared_from_this<MemoryPool> {
	friend MemoryBlock;

public:
	MemoryPool(size_t size, uint32_t caps, size_t count = 0);
	~MemoryPool() = default;

	bool resize(size_t count);
	std::unique_ptr<MemoryBlock> allocate();

private:
	static uuid::log::Logger logger_;

	MemoryPool(MemoryPool&&) = delete;
	MemoryPool(const MemoryPool&) = delete;
	MemoryPool& operator=(MemoryPool&&) = delete;
	MemoryPool& operator=(const MemoryPool&) = delete;

	void restore(MemoryAllocation data);

	const size_t size_;
	const uint32_t caps_;
	std::mutex mutex_;
	std::vector<MemoryAllocation> blocks_;
	size_t used_{0};
	size_t capacity_{0};
};

} // namespace aurcor
