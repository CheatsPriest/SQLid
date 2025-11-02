#pragma once
#include "parser/Query.h"
#include "InfoLoader.h"
#include "parser/Conditions.h"
#include <variant>
#include <supported_variants.h>


//I am happy with the encapsulation violation
class Optimizer {
private:
	template<typename T>
	void optimizeConditions(QueryBase<T>& query, std::shared_ptr<TabbleInfo>& info) {

		std::string columnName;
		std::string oper;
		std::string valueStr;
		size_t columnId = 0;
		Type columnType = Type::VOIDA;
		variant_types desired;
		ConditionType conditionType;

		auto& raw_conditions = query.raw_conditions;
		auto& conditions = query.conditions;

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
				Condition cur(conditionType, SIZE_MAX, 0);

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
					Condition cur(ConditionType::DIRECT_INDEX, 0, target_id);
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

	template<typename T>
	void optimizeColumns(T& query, std::shared_ptr<TabbleInfo>& info) {
		for (auto& col : query.columns_raw) {
			query.columns_optimized.push_back(info->columns_map.at(col));
		}
		//columns_raw.clear(); 
	}

	template<typename T>
	void optimizeInsertValues(T& query, std::shared_ptr<TabbleInfo>& info) {

		size_t i = 0;
		for (Column& column : info->columns) {
			query.values.push_back(parse_value(query.raw_values.at(i), column.type));
			++i;
		}

	}
	
	void optimizeUpdateValues(UpdateQuery& query, std::shared_ptr<TabbleInfo>& info) {

		size_t i = 0;
		for (size_t id : query.columns_optimized) {
			query.values.push_back(parse_value(query.raw_values.at(i), info->columns[id].type));
			++i;
		}

	}

public:

	
	
	void optimize(QueryVariant& query, std::shared_ptr<TabbleInfo> info, Tabble& table) {

		std::visit([&info, &table, this](auto& query) {
			using T = std::decay_t<decltype(query)>;
			query.table_ptr = &table;
			if constexpr (std::is_same_v<T, SelectQuery>) {
				optimizeConditions(query, info);
				optimizeColumns(query, info);
			}
			if constexpr (std::is_same_v<T, InsertQuery>) {
				optimizeInsertValues(query, info);
			}
			if constexpr (std::is_same_v<T, DeleteQuery>) {
				optimizeConditions(query, info);
			}
			if constexpr (std::is_same_v<T, UpdateQuery>) {
				optimizeColumns(query, info);
				optimizeConditions(query, info);
				optimizeUpdateValues(query, info);
			}
			//חהוסב update

			}, query);

	}

};