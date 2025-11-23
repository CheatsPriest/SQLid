#pragma once
#include "data_base/DataBase.h"
#include <string>
#include <memory>
#include <vector>
#include <filesystem>
#include <shared_mutex>

namespace fs = std::filesystem;

static const std::string backUpName = "backup";

class System {
private:
	
	std::string abosolutePath;

	std::vector<std::unique_ptr<DataBase>> bases;
	std::unordered_map<std::string, size_t> name_base;
	std::shared_mutex bases_mtx;


	void loadFoldersAndOpenBases() {
		try {
			bool hasNotBackUp=true;

			bases.reserve(8);
			size_t i = 0;

			if (std::filesystem::is_empty(abosolutePath)) {
				//std::cout << "Creating default database..." << std::endl;
				createDataBase("default");
				//std::filesystem::create_directory("default");
			}

			for (const auto& entry : std::filesystem::directory_iterator(abosolutePath)) {
				if (entry.is_directory()) {
					if (hasNotBackUp and entry.path().filename() == backUpName) {
						hasNotBackUp = false;
					}
					try {
						auto base = std::make_unique<DataBase>(abosolutePath + "/" + entry.path().filename().string(), entry.path().filename().string());
						bases.push_back(std::move(base));
						name_base.insert({ entry.path().filename().string() , i });
						++i;
					}
					catch (...) {

					}
				}
			}
			if (hasNotBackUp) {
				std::filesystem::create_directory(abosolutePath+"/"+backUpName);
				std::cout << "Backup created" << std::endl;
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

		auto base = std::make_unique<DataBase>(abosolutePath + "/" + name, name);

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
	void backUpDatabase(std::string_view dbName, std::string_view backupName) {
		std::shared_lock<std::shared_mutex> lock(bases_mtx);

		auto it = name_base.find(std::string(dbName));
		if (it == name_base.end()) {
			throw IncorrectInputException("Database '" + std::string(dbName) + "' not found");
		}

		std::string backupStr(backupName);
		if (backupStr.empty() || backupStr.find_first_of("/\\?%*:|\"<>") != std::string::npos) {
			throw IncorrectInputException("Invalid backup name");
		}

		fs::path source_path = fs::path(abosolutePath) / dbName;
		fs::path backup_path = fs::path(abosolutePath) / backUpName / backupStr;

		if (!fs::exists(source_path) || !fs::is_directory(source_path)) {
			throw IncorrectInputException("Source database directory not found: " + source_path.string());
		}

		if (!fs::create_directory(backup_path)) {
			throw IncorrectInputException("Failed to create backup directory: " + backup_path.string());
		}

		try {
			for (const auto& entry : fs::recursive_directory_iterator(source_path)) {
				fs::path relative_path = fs::relative(entry.path(), source_path);
				fs::path target_path = backup_path / relative_path;

				if (entry.is_directory()) {
					fs::create_directories(target_path);
				}
				else {
					fs::copy_file(entry.path(), target_path, fs::copy_options::overwrite_existing);
				}
			}

			std::cout << "Backup '" << backupStr << "' created successfully for database '" << dbName << "'" << std::endl;

		}
		catch (const fs::filesystem_error& e) {
			fs::remove_all(backup_path);
			throw IncorrectInputException("Backup failed: " + std::string(e.what()));
		}
	}

	size_t deleteDatabase(const std::string& dbName) {
		std::unique_lock<std::shared_mutex> lock(bases_mtx);  

		size_t id;
		try {
			id = name_base.at(dbName);  
		}
		catch (...) {
			throw IncorrectInputException("Database '" + dbName + "' not found");
		}

		bases[id].reset(nullptr);
		name_base.erase(dbName);

		fs::path db_path = fs::path(abosolutePath) / dbName;
		try {
			if (fs::exists(db_path) && fs::is_directory(db_path)) {
				fs::remove_all(db_path);
				std::cout << "Database directory '" << dbName << "' deleted successfully" << std::endl;
			}
		}
		catch (const fs::filesystem_error& e) {
			throw IncorrectInputException("Failed to delete database directory: " + std::string(e.what()));
		}

		std::cout << "Database '" << dbName << "' deleted successfully from system" << std::endl;
		return id;
	}

	void restoreDatabase(std::string_view backupName, std::string_view dbName) {
		std::unique_lock<std::shared_mutex> lock(bases_mtx);  

		fs::path backup_path = fs::path(abosolutePath) / backUpName / backupName;
		fs::path restore_path = fs::path(abosolutePath) / dbName;

		if (!fs::exists(backup_path) || !fs::is_directory(backup_path)) {
			throw IncorrectInputException("Backup '" + std::string(backupName) + "' not found");
		}

		size_t id;
		bool database_existed = false;

		auto it = name_base.find(std::string(dbName));
		if (it != name_base.end()) {
			database_existed = true;
			id = it->second;
			bases[id].reset(nullptr);
		}
		else {
			id = bases.size();
		}

		try {
			
			if (fs::exists(restore_path)) {
				fs::remove_all(restore_path);
			}

			fs::create_directory(restore_path);

			
			for (const auto& entry : fs::recursive_directory_iterator(backup_path)) {
				fs::path relative_path = fs::relative(entry.path(), backup_path);
				fs::path target_path = restore_path / relative_path;

				if (entry.is_directory()) {
					fs::create_directories(target_path);
				}
				else {
					fs::copy_file(entry.path(), target_path, fs::copy_options::overwrite_existing);
				}
			}

		
			auto base = std::make_unique<DataBase>(restore_path.string(), std::string(dbName));

			if (database_existed) {
				bases[id] = std::move(base); 
			}
			else {
				bases.push_back(std::move(base));  
				name_base.insert({ std::string(dbName), id });
			}

			std::cout << "Database '" << dbName << "' restored successfully from backup '" << backupName << "'" << std::endl;

		}
		catch (const fs::filesystem_error& e) {
			
			fs::remove_all(restore_path);
			throw IncorrectInputException("Restore failed: " + std::string(e.what()));
		}
	}
	const auto& getBases() {
	
		return bases;
	}
	auto& getMtx() {
		return bases_mtx;
	}
};