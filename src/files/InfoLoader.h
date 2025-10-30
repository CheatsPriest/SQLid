#pragma once

#include <fstream>
#include <vector>
#include <stdexcept>
#include <array>
#include <unordered_map>
#include <atomic>
#include <thread>



enum class Type { VOIDA = 0, INT32 = 1, INT64 = 2, DOUBLE= 3, FLOAT = 4, BOOL = 5, TEXT =6, STRING = 7 };


static constexpr std::array<size_t, 8> type_sizes = {
	0,  // VOIDA
	sizeof(int),  // INT32
	sizeof(long long),  // INT64  
	sizeof(double),  // DOUBLE
	sizeof(float),  // FLOAT
	sizeof(bool),  // BOOL
	128,  // TEXT 
	0   // STRING - User decides on his own size
};

struct Column
{
	
	std::string name;
	size_t offset;
	Type type;
	size_t size;
	Column() : name(""), offset(0), type(Type::VOIDA), size(0) {};
};

struct DataBaseInfo
{
	size_t lineLength;
	size_t columnsNumber;
	std::vector<Column> columns;
	DataBaseInfo() : lineLength(0), columnsNumber(0), columns(0) {};
};

class InfoLoader {
private:

	std::unordered_map<std::string, Type> stringToType = {
			{"VOID", Type::VOIDA}, {"INT32", Type::INT32},
			{"INT64", Type::INT64}, {"DOUBLE", Type::DOUBLE},
			{"FLOAT", Type::FLOAT}, {"BOOL", Type::BOOL},
			{"TEXT", Type::TEXT}, {"STRING", Type::STRING}
	};

	std::ifstream  in; // поток для чтения
	std::string& name;
	std::atomic<std::shared_ptr<DataBaseInfo>> info;
	std::atomic<bool> loading{ false };

	bool loadFile() {

		bool expected = false;

		if (!loading.compare_exchange_strong(expected, true)) {

			loading.wait(true, std::memory_order_acquire); 
			

			return true;
		}



		DataBaseInfo newInfo;
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

		info.store(std::make_shared<DataBaseInfo>(newInfo), std::memory_order_release);

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
	}

	std::shared_ptr<DataBaseInfo> loadNewInfo() {
		loadFile();
		return info.load(std::memory_order_acquire);
	}

	std::shared_ptr<DataBaseInfo> getCurrentInfo() const {
		return info.load(std::memory_order_acquire);
	}


	~InfoLoader() {
		in.close();
	}
};