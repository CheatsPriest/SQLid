#pragma once
#include "data_base/DataBase.h"
#include <string>
#include <memory>
#include <vector>
#include <filesystem>
#include <shared_mutex>

namespace fs = std::filesystem;

class System {
private:
	
	std::string abosolutePath;

	std::vector<std::unique_ptr<DataBase>> bases;
	std::unordered_map<std::string, size_t> name_base;
	std::shared_mutex bases_mtx;


	void loadFoldersAndOpenBases() {
		try {
			bases.reserve(8);
			size_t i = 0;

			if (std::filesystem::is_empty(abosolutePath)) {
				//std::cout << "Creating default database..." << std::endl;
				createDataBase("default");
				//std::filesystem::create_directory("default");
			}

			for (const auto& entry : std::filesystem::directory_iterator(abosolutePath)) {
				if (entry.is_directory()) {
					try {
						auto base = std::make_unique<DataBase>(abosolutePath + "/" + entry.path().filename().string());
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
		std::shared_lock<std::shared_mutex> lock(bases_mtx);
		return *bases.at(name_base.at(name));
	}
	DataBase& getDataBaseById(size_t ind) {
		std::shared_lock<std::shared_mutex> lock(bases_mtx);
		return *bases.at(ind);
	}
	size_t getDataBaseId(const std::string& name) {
		std::shared_lock<std::shared_mutex> lock(bases_mtx);
		return name_base.at(name);
	}
	void createDataBase(const std::string& name) {
		

		if (name.empty()) {
			throw IncorrectInputException("Database name cannot be empty");
		}

		if (name.find_first_of("/\\?%*:|\"<>") != std::string::npos) {
			throw IncorrectInputException("Database name contains invalid characters");
		}

		
		fs::path db_path = fs::path(abosolutePath) / name;

		
		if (fs::exists(db_path)) {
			throw IncorrectInputException("Database '" + name + "' already exists");
		}

		if (!fs::create_directory(db_path)) {
			throw IncorrectInputException("Failed to create database directory: " + db_path.string());
		}

		auto base = std::make_unique<DataBase>(abosolutePath + "/" + name);

		std::unique_lock<std::shared_mutex> lock(bases_mtx);
		size_t id = bases.size();
		bases.push_back(std::move(base));
		name_base.insert({ name , id });
	}
	void createTable(std::stringstream& stream) {
		std::string buf;
		stream >> buf;
		auto& base = getDataBase(buf);
		base.createTable(stream);
	}
};