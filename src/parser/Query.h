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

	void writeColumns() {

	}

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

private:
private:
	// Чтение одной записи
	std::vector<variant_types> readRecord(Tabble& table, size_t line_id,
		const std::vector<size_t>& column_ids,
		const std::vector<Column>& columns) {
		std::vector<variant_types> record;
		record.reserve(column_ids.size() + 1);
		record.push_back((int64_t)line_id); // ID записи

		for (size_t col_id : column_ids) {
			const auto& col = columns[col_id];
			if (col.type == Type::TEXT || col.type == Type::STRING) {
				record.push_back(std::string(table.readText(line_id, col.offset, col.size)));
			}
			else {
				record.push_back(table.readNumber(line_id, col.offset, col.type));
			}
		}
		return record;
	}

	//  Проверка условий
	bool satisfiesConditions(Tabble& table, size_t line_id,
		const std::vector<Condition>& conditions,
		const std::vector<Column>& columns) {
		bool current_result = false;
		ConditionType logic_op = ConditionType::LOGICAL_OR;

		for (const auto& cond : conditions) {
			if (cond.collumn_id == SIZE_MAX) {
				logic_op = cond.cond_type; // AND/OR
				continue;
			}

			const auto& col = columns[cond.collumn_id];
			bool condition_met = false;

			if (col.type == Type::TEXT || col.type == Type::STRING) {
				auto record_value = table.readText(line_id, col.offset, col.size);
				condition_met = (std::get<std::string>(cond.desired) == record_value);
			}
			else {
				auto record_value = table.readNumber(line_id, col.offset, col.type);
				condition_met = (cond.desired == record_value);
			}

			// Применяем логический оператор
			if (logic_op == ConditionType::LOGICAL_OR) {
				current_result = current_result || condition_met;
			}
			else { // LOGICAL_AND
				current_result = current_result && condition_met;
			}
		}

		return current_result;
	}

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

	//I know what you will say. I know that it is bad function.
	Result executeSelect(Tabble& table, std::shared_ptr<TabbleInfo>& info) {
		Result result;
		auto& storage = table();
		auto& columns = info->columns;

		// ?? DIRECT INDEX - O(1) оптимизация
		if (!conditions.empty() && conditions[0].cond_type == ConditionType::DIRECT_INDEX) {
			int64_t line_id = std::get<int64_t>(conditions[0].desired);
			result.body.push_back(readRecord(table, line_id, columns_optimized, columns));
			return result;
		}

		//  FULL SCAN с фильтрацией
		size_t max_line = min(storage.getMaxLines(), limit);
		size_t active_count = table.getActivePlaces();

		result.body.reserve(active_count);

		for (int64_t i = 0; i < max_line && result.body.size() < active_count; ++i) {
			if (!storage.isActive(i)) continue;
			if (!satisfiesConditions(table, i, conditions, columns)) continue;

			result.body.push_back(readRecord(table, i, columns_optimized, columns));
		}

		return result;
	}
	
	Result execute(Tabble& tabble, std::shared_ptr<TabbleInfo>& info) {
		
		//this->executeInsert(tabble, info);


		return this->executeSelect(tabble, info);
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