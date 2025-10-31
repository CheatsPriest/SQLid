#pragma once

#include <vector>
#include <string>
#include "Conditions.h"
#include <memory>
#include "files/InfoLoader.h"
#include "files/Tabble.h"
#include "result/Result.h"
#include <format>


enum class QueryType { SELECT, UPDATE, INSERT, ERASE };



template<typename Derived>
class QueryBase {
public:
    std::string tabble_name;
    std::vector<Condition> conditions;
    std::vector<std::string> raw_conditions;
    size_t limit = SIZE_MAX;

    // Методы которые будут у всех запросов

    void optimizeConditions(std::shared_ptr<TabbleInfo> info) {

		std::string columnName;
		std::string oper;
		std::string valueStr;
		size_t columnId = 0;
		Type columnType = Type::VOIDA;
		variant_types desired;
		ConditionType conditionType;


		for (size_t i = 0; i < raw_conditions.size();) {

			if (raw_conditions[i] == "AND" or raw_conditions[i] == "OR") {
				oper = std::move(raw_conditions[i]);
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

				conditions.push_back(std::move(cur));
				++i;
			}
			else {
				try {
					columnName = std::move(raw_conditions.at(i));
					oper = std::move(raw_conditions.at(i + 1));
					valueStr = std::move(raw_conditions.at(i + 2));
				}
				catch (std::out_of_range& erorr) {
					throw std::out_of_range("Incorrect input");
				}

				if (columnName == "id" or columnName == "ID") {
					variant_types target_id = static_cast<int64_t>(std::stoul(valueStr));
					Condition cur(ConditionType::DIRECT_INDEX, 0,  target_id );
					conditions.push_back(std::move(cur));
				}
				else {
					try {
						columnId = info->columns_map.at(columnName);
					}
					catch (std::runtime_error& error) {
						throw std::runtime_error("Invalid column name");
					}

					columnType = info->columns[columnId].type;



					const auto& column = info->columns[columnId];
					variant_types desired_value = std::move(parse_value(valueStr, column.type));

					try {
						conditionType = condition_map.at(oper);
					}
					catch (std::runtime_error& error) {
						throw std::runtime_error("Incorrect operation");
					}

					Condition cur(conditionType, columnId, desired_value);

					conditions.push_back(std::move(cur));
				}
				i += 3;
			}
		}
    }

    // CRTP trick - доступ к производному классу
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }
};



class SelectQuery : public QueryBase<SelectQuery> {
public:
    std::vector<std::string> columns_raw;
    std::vector<size_t> columns_optimized;

    
	void optimizeColumns(std::shared_ptr<TabbleInfo> info) {
		for (auto& col : columns_raw) {
			columns_optimized.push_back(info->columns_map.at(col));
		}
		//columns_raw.clear(); 
	}
	void optimizeImpl(std::shared_ptr<TabbleInfo> info) {
		this->optimizeConditions(info);  // Из базового
		this->optimizeColumns(info);     // Своя логика
	}
	
	Result execute(Tabble& tabble, std::shared_ptr<TabbleInfo>& info) {
		Result cur;
		//this->executeInsert(tabble, info);


		return cur;
	}
};


class InsertQuery : public QueryBase<InsertQuery> {
public:
    std::vector<std::string> raw_values;
    std::vector<variant_types> values;

	void optimizeValues(std::shared_ptr<TabbleInfo> info){

		size_t i = 0;
		for (Column& column : info->columns) {
			values.push_back(parse_value(raw_values.at(i), column.type));
			++i;
		}

	}

	void optimizeImpl(std::shared_ptr<TabbleInfo> info) {
		this->optimizeConditions(info);  // Из базового
		this->optimizeValues(info);
	}

	

	Result executeInsert(Tabble& tabble, std::shared_ptr<TabbleInfo>& info) {

		Result cur;
		try {
			cur.messeage = std::format("Inserted into {}", tabble.insert(values, info->columns));


			cur.isSucces = true;
		}
		catch (std::runtime_error& error) {
			cur.error = std::move(error.what());
		}
		catch (std::exception& error) {
			cur.error = std::move(error.what());
		}
		return cur;
	}

	Result execute(Tabble& tabble, std::shared_ptr<TabbleInfo>& info) {
		
		return executeInsert(tabble, info);


	}

};


using QueryVariant = std::variant<
    SelectQuery,
    //UpdateQuery,
    InsertQuery
    //DeleteQuery
>;