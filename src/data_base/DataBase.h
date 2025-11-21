#pragma once
#include "files/Tabble.h"
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include "files/Optimizer.h"
#include <filesystem>

//TO DO: Добавить подгрузку данных самой базы данных при запуске
class DataBase {
private:

	std::string absoluteWay;
	std::string dataBaseName;

	Optimizer optim;

	std::vector<std::unique_ptr<Tabble>> tabbles;

	
	std::unordered_map<std::string, size_t> tabbles_name;
	mutable std::shared_mutex map_mtx;

	size_t getTabbleId(const std::string& name) const {
		std::shared_lock<std::shared_mutex> lock(map_mtx);
		try {
			return tabbles_name.at(name);
		}
		catch(...){
			throw IncorrectInputException("Unknown table name: "+name);
		}
	}

	void insertTabble(std::unique_ptr<Tabble> tabble, const std::string& name) {
		std::unique_lock<std::shared_mutex> lock(map_mtx);
		tabbles_name.insert({ name, tabbles.size() });
		tabbles.push_back(std::move(tabble));
	}

	void loadFoldersAndOpenTabbles() {
		try {
			for (const auto& entry : std::filesystem::directory_iterator(absoluteWay)) {
				if (entry.is_directory()) {
					openTabble(entry.path().filename().string());
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			throw e;
		}
	}

public:
	
	DataBase(std::string absoluteWay_) : absoluteWay(absoluteWay_) {

		tabbles.reserve(16);
		loadFoldersAndOpenTabbles();
	};

	bool openTabble(std::string localName) {
		
		std::cout << localName << std::endl;

		auto cur = std::make_unique<Tabble>(absoluteWay + "/" + localName + "/" + localName);
		
		insertTabble(std::move(cur), localName);

		return true;
	}


	void optimizing(QueryVariant& query_var) {

		std::string tabble_name = std::visit([](auto&& query) -> std::string {
			return query.tabble_name;  // работает благодаря CRTP
			}, query_var);

		size_t id = getTabbleId(tabble_name);

		Tabble& tabble = *tabbles[id];
		auto info = tabble.getInfo();

		optim.optimize(query_var, tabble.getInfo(), tabble);

	}



    void createTable(std::stringstream& stream) {
        namespace fs = std::filesystem;

        std::string table_name;
        stream >> table_name;

        if (table_name.empty() || table_name.find_first_of("/\\?%*:|\"<>") != std::string::npos) {
            throw IncorrectInputException("Invalid table name: " + table_name);
        }

        if (tabbles_name.find(table_name) != tabbles_name.end()) {
            throw IncorrectInputException("Table '" + table_name + "' already exists");
        }

        std::vector<std::string> column_names;
        std::vector<Type> column_types;
        std::vector<size_t> column_sizes;

        std::string col_name, col_type;
        while (stream >> col_name >> col_type) {
            column_names.push_back(col_name);

            
            if (col_type.find("STRING[") == 0 && col_type.back() == ']') {
                size_t string_size = std::stoul(col_type.substr(7, col_type.length() - 8));
                column_types.push_back(Type::STRING);
                column_sizes.push_back(string_size);
            }
            else {
                
                auto it = stringToType.find(col_type);
                if (it == stringToType.end()) {
                    throw IncorrectInputException("Unknown type: " + col_type);
                }
                column_types.push_back(it->second);
                column_sizes.push_back(type_sizes[static_cast<int>(it->second)]);
            }
        }

        if (column_names.empty()) {
            throw IncorrectInputException("No columns specified for table: " + table_name);
        }

        
        size_t line_length = sizeof(bool); // isActive
        for (size_t size : column_sizes) {
            line_length += size;
        }
        line_length *= 3/2;

        
        fs::path table_path = fs::path(absoluteWay) / table_name;
        if (!fs::create_directory(table_path)) {
            throw IncorrectInputException("Failed to create table directory: " + table_path.string());
        }

     
        std::string info_path = absoluteWay + "/" + table_name + "/" + table_name + "_info.db";
        std::ofstream info_file(info_path);

        // Записываем мета-информацию
        info_file << line_length << "\n";
        info_file << column_names.size() << "\n";

        // Имена колонок
        for (size_t i = 0; i < column_names.size(); ++i) {
            info_file << column_names[i] << (i < column_names.size() - 1 ? " " : "\n");
        }

        for (size_t i = 0; i < column_types.size(); ++i) {
            if (i > 0) info_file << " ";  // пробел между типами

            info_file << typeToString[static_cast<int>(column_types[i])];
            if (column_types[i] == Type::STRING) {
                info_file << " " << column_sizes[i];
            }
        }
        info_file << "\n";

        info_file.close();

        std::string storage_path = absoluteWay + "/" + table_name + "/" + table_name + "_storage.db";
        std::ofstream storage_file(storage_path, std::ios::binary);
        storage_file.close();
        openTabble(table_name);

       
    }
	
};