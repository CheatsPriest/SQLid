#pragma once
#include "data_base/DataBase.h"
#include <string>
#include <memory>
#include <vector>

class System {
private:
	std::string abosolutePath;

	std::vector<std::unique_ptr<DataBase>> bases;
	std::unordered_map<std::string, size_t> name_base;

	void loadFoldersAndOpenBases() {
		try {
			bases.reserve(8);
			size_t i = 0;
			for (const auto& entry : std::filesystem::directory_iterator(abosolutePath)) {
				if (entry.is_directory()) {
					try {
						auto base = std::make_unique<DataBase>(abosolutePath + "\\" + entry.path().filename().string());
						bases.push_back(std::move(base));
						name_base.insert({ entry.path().filename().string() , i });
						++i;
					}
					catch (...) {

					}
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			throw e;
		}
	}

public:

	System(std::string abosolutePath_) : abosolutePath(abosolutePath_) {

		if (!std::filesystem::exists(abosolutePath)) {
			throw std::runtime_error("Path does not exist: " + abosolutePath);
		}
		if (!std::filesystem::is_directory(abosolutePath)) {
			throw std::runtime_error("Path is not a directory: " + abosolutePath);
		}

		loadFoldersAndOpenBases();
	}
	DataBase& getDataBase(const std::string& name) {
		return *bases.at(name_base.at(name));
	}
	DataBase& getDataBaseById(size_t ind) {
		return *bases.at(ind);
	}
	size_t getDataBaseId(const std::string& name) {
		return name_base.at(name);
	}
};