#pragma once
#include "files/Tabble.h"
#include <vector>
#include <unordered_map>
#include <shared_mutex>


//TO DO: Добавить подгрузку данных самой базы данных при запуске
class DataBase {
private:

	variant_types parse_value(const std::string& str, Type type) {
		

		switch (type) {
		case Type::INT32: return variant_types{ static_cast<int32_t>(std::stoi(str)) };
		case Type::INT64: return variant_types{ static_cast<int64_t>(std::stoll(str)) };
		case Type::DOUBLE: return variant_types{ static_cast<double>(std::stod(str)) };
		case Type::FLOAT: return variant_types{ static_cast<float>(std::stof(str)) };
		case Type::BOOL:
			if (str == "true" or str == "1") return variant_types{ true };
			if (str == "false" or str == "0") return variant_types{ false };
			throw std::runtime_error("Invalid boolean value: " + str);
		case Type::STRING:
		case Type::TEXT:
			return variant_types{ str }; // Уже строка
		default:
			throw std::runtime_error("Unsupported type");
		}
	}

	std::vector<std::unique_ptr<Tabble>> tabbles;

	
	std::unordered_map<std::string, size_t> tabbles_name;
	mutable std::shared_mutex map_mtx;

	size_t getTabbleId(const std::string& name) const {
		std::shared_lock<std::shared_mutex> lock(map_mtx);
		return tabbles_name.at(name);
		
	}

	void insertTabble(std::unique_ptr<Tabble> tabble, const std::string& name) {
		std::unique_lock<std::shared_mutex> lock(map_mtx);
		tabbles_name.insert({ name, tabbles.size() });
		tabbles.push_back(std::move(tabble));
	}

public:

	DataBase() {

		tabbles.reserve(16);

	};

	bool openTabble(std::string filename, std::string localName) {

		auto cur = std::make_unique<Tabble>(filename);
		
		insertTabble(std::move(cur), localName);

		return true;
	}

	void optimizeQuery(Query& in) {

		size_t id = 0;
		try {
			id = getTabbleId(in.table_name);
		}
		catch(std::exception& error){
			throw error;
		}
		
		Tabble& cur = *tabbles[id];

		auto info = cur.getInfo();
		
		for (auto& el : in.columns_raw) {
			in.columns_optimized.push_back(info->columns_map[el]);
		}
		//in.columns_raw.clear(); //Is It Optimal?
		//
		

		std::string columnName;
		std::string oper;
		std::string valueStr;
		size_t columnId=0;
		Type columnType = Type::VOIDA;
		variant_types desired;
		ConditionType conditionType;


		for (size_t i = 0; i < in.raw_conditions.size();) {

			if (in.raw_conditions[i] == "AND" or in.raw_conditions[i] == "OR") {
				oper = std::move(in.raw_conditions[i]);
				if (oper == "AND") {
					conditionType = ConditionType::LOGICAL_AND;
				}
				else if (oper == "OR") {
					conditionType = ConditionType::LOGICAL_OR;
				}
				else {
					throw std::runtime_error("Incoreect operation");
				}
				Condition cur(conditionType, 0, 0);

				in.conditions.push_back(std::move(cur));
				++i;
			}
			else {
				try {
					columnName = std::move(in.raw_conditions.at(i));
					oper = std::move(in.raw_conditions.at(i + 1));
					valueStr = std::move(in.raw_conditions.at(i + 2));
				}
				catch (std::out_of_range& erorr) {
					throw std::out_of_range("Incorrect input");
				}


				try {
					columnId = info->columns_map.at(columnName);
				}
				catch(std::runtime_error& error){
					throw std::runtime_error("Invalid column name");
				}

				columnType = info->columns[columnId].type;

				

				const auto& column = info->columns[columnId];
				variant_types desired_value = std::move(parse_value(valueStr, column.type));

				try{
					conditionType = condition_map.at(oper);
				}
				catch(std::runtime_error& error){
					throw std::runtime_error("Incorrect operation");
				}

				Condition cur(conditionType, columnId, desired_value);

				in.conditions.push_back(std::move(cur));
				i += 3;
			}
		}

	}

};