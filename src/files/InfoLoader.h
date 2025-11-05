#pragma once

#include <fstream>
#include <vector>
#include <stdexcept>
#include <array>
#include <unordered_map>
#include <atomic>
#include <thread>
#include "supported_variants.h"







struct Column
{
	
	std::string name;
	size_t offset;
	Type type;
	size_t size;
	Column() : name(""), offset(1), type(Type::VOIDA), size(0) {};
};

struct TabbleInfo
{
	size_t lineLength;
	size_t columnsNumber;
	std::vector<Column> columns;
	std::unordered_map<std::string, size_t> columns_map;

	TabbleInfo() : lineLength(0), columnsNumber(0), columns(0) {};
};

class InfoLoader {
private:

	

	std::ifstream  in; // поток для чтения
	std::string& name;
	std::atomic<std::shared_ptr<TabbleInfo>> info;
	std::atomic<bool> loading{ false };

	bool loadFile() {

		bool expected = false;

		if (!loading.compare_exchange_strong(expected, true)) {

			loading.wait(true, std::memory_order_acquire); 
			

			return true;
		}



		TabbleInfo newInfo;
		in >> newInfo.lineLength;
		in >> newInfo.columnsNumber;
		newInfo.columns.resize(newInfo.columnsNumber);

		for (size_t i = 0; i < newInfo.columnsNumber; ++i) {
			in >> newInfo.columns[i].name;
		}
		std::string curType;
		for (size_t i = 0; i < newInfo.columnsNumber; ++i) {
			in >> curType;

			if(curType!="STRING"){
				newInfo.columns[i].type = stringToType[curType];
				newInfo.columns[i].size = type_sizes[(size_t)newInfo.columns[i].type];

			}
			else {
				size_t string_size;
				in >> string_size;
				newInfo.columns[i].type = stringToType[curType];
				newInfo.columns[i].size = string_size;
			}
			if (i > 0) {
				newInfo.columns[i].offset = newInfo.columns[i - 1].offset + newInfo.columns[i - 1].size;
			}
		}

		size_t ind = 0;
		for (Column& c : newInfo.columns) {
			newInfo.columns_map.insert({ c.name, ind++ });
		}

		info.store(std::make_shared<TabbleInfo>(newInfo), std::memory_order_release);

		loading.store(false, std::memory_order_release);

		loading.notify_all();

		return true;
	}

public:

	InfoLoader(std::string& name_) : name(name_) {
		in.open(name.c_str());
		if (!in.is_open()) {
			throw std::runtime_error("Can not open the info file");
		}
		loadFile();
	}

	InfoLoader(const InfoLoader&) = delete;            // Явно запретить копирование
	InfoLoader& operator=(const InfoLoader&) = delete;
	InfoLoader(InfoLoader&&) = default;                // Разрешить перемещение
	InfoLoader& operator=(InfoLoader&&) = default;

	std::shared_ptr<TabbleInfo> loadNewInfo() {
		loadFile();
		return info.load(std::memory_order_acquire);
	}

	std::shared_ptr<TabbleInfo> getCurrentInfo() const {
		return info.load(std::memory_order_acquire);
	}


	~InfoLoader() {
		in.close();
	}
};