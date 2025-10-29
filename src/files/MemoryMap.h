#pragma once

#include <string>
#include <memory>
#include "MemoryMapWindows.h"
#include <shared_mutex>
#include <array>
#include <vector>

class MemoryMap {
private:
	static constexpr size_t STRIPE_COUNT = 128;
	std::array<std::shared_mutex, STRIPE_COUNT> stripes;
	
	std::shared_mutex expand_end_mutex;

	size_t get_stripe(size_t id) const {
		return id % STRIPE_COUNT;
	}
	std::atomic<bool> expanding{ false };

	bool expand() {
		bool expected = false;
		if (!expanding.compare_exchange_strong(expected, true)) {
			while (expanding.load(std::memory_order_acquire)) {
				std::this_thread::yield();
			}
			return false; // Уже кто-то расширяет
		}
		

		std::vector<std::unique_lock<std::shared_mutex>> locks(STRIPE_COUNT);
		for (auto& stripe : stripes) {
			locks.emplace_back(stripe);
		}

		bool result = core.expandFile();
		expanding.store(false);

		return result;
	}

public:
	MemoryMapWindows core;
	MemoryMap(const std::string& filename, size_t lineLength)
		: core(filename, lineLength) {
	}
	~MemoryMap() = default; // unique_ptr сам уничтожит Impl


	
	MemoryMap(const MemoryMap&) = delete;
	MemoryMap& operator=(const MemoryMap&) = delete;

	
	MemoryMap(MemoryMap&&) noexcept = default;
	MemoryMap& operator=(MemoryMap&&) noexcept = default;
	
	template<typename T>
	void writeNumber(size_t lineNumber, size_t offset, T value) {
		if (core.getMaxLine() <= lineNumber) {
			expand();
			
		}
		std::unique_lock<std::shared_mutex> lock(stripes[get_stripe(lineNumber)]);
		core.writeNumber<T>(lineNumber, offset, value);
	}

	template<typename T>
	T readNumber(size_t lineNumber, size_t offset = 0) {
		std::shared_lock<std::shared_mutex> lock(stripes[get_stripe(lineNumber)]);

		return core.readNumber<T>(lineNumber, offset);
	}

	void writeText(size_t lineNumber, size_t offset, std::string_view value) {
		if (core.getMaxLine() <= lineNumber) {
			expand();
		}
		std::unique_lock<std::shared_mutex> lock(stripes[get_stripe(lineNumber)]);
		core.writeText(lineNumber, offset, value);
	}
	std::string_view readText(size_t lineNumber, size_t offset, size_t size) {
		std::shared_lock<std::shared_mutex> lock(stripes[get_stripe(lineNumber)]);
		return core.readText(lineNumber, offset, size);
	}

private:
	



};

