#pragma once

#include "parser/Query.h"
#include "result/Result.h"
#include <format>

class Executor {
private:

	bool satisfiesConditions(Tabble& table, size_t line_id,
		std::vector<Condition>& conditions,
		std::vector<Column>& columns) {
		bool current_result = false;
		ConditionType logic_op = ConditionType::LOGICAL_OR;

		for (Condition& cond : conditions) {
			if (cond.collumn_id == SIZE_MAX) {
				logic_op = cond.cond_type; // AND/OR
				continue;
			}



			const auto& col = columns[cond.collumn_id];
			bool condition_met = false;

			if (col.type == Type::TEXT or col.type == Type::STRING) {
				auto record_value = table.readText(line_id, col.offset, col.size);
				condition_met = cond.result(std::string(record_value));
			}
			else {
				auto record_value = table.readNumber(line_id, col.offset, col.type);
				condition_met = cond.result(record_value);
			}

			// Применяем логический оператор
			if (logic_op == ConditionType::LOGICAL_OR) {
				current_result = current_result or condition_met;
			}
			else { // LOGICAL_AND
				current_result = current_result and condition_met;
			}
		}

		return current_result;
	}
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


	void executeSelect(SelectQuery& query, Result& result) {

		result.messeage = "Selected";

		auto& table = *query.table_ptr;
		auto& storage = table.getStoarge();
		auto info = table.getInfo();
		auto& columns = info->columns;
		auto& conditions = query.conditions;
		auto& columns_optimized = query.columns_optimized;
		
		result.header.reserve(columns_optimized.size()+1);
		result.types.reserve(columns_optimized.size()+1);
		result.header.push_back("ID");
		result.types.push_back("INT64");
		for (size_t ind : columns_optimized) {
			result.header.push_back(columns[ind].name);
			result.types.push_back(typeToString[(size_t)columns[ind].type]);
		}



		//  DIRECT INDEX - O(1) оптимизация
		if (!conditions.empty() and conditions[0].cond_type == ConditionType::DIRECT_INDEX) {
			int64_t line_id = std::get<int64_t>(conditions[0].desired);
			result.body.push_back(readRecord(table, line_id, columns_optimized, columns));
			return;
		}

		//  FULL SCAN с фильтрацией
		size_t max_line = table.getMaxActiveLine();
		
		size_t active_count = table.getActivePlaces();
		active_count = active_count < query.limit ? active_count : query.limit;

		result.body.reserve(active_count);

		for (int64_t i = 0; i < max_line && result.body.size() < active_count; ++i) {
			if (!storage.isActive(i)) continue;
			if (!conditions.empty() and !satisfiesConditions(table, i, conditions, columns)) continue;

			result.body.push_back(readRecord(table, i, columns_optimized, columns));
		}
	}

	void executeInsert(InsertQuery& query, Result& result) {

		auto& table = *query.table_ptr;
		auto info = table.getInfo();
		result.header.push_back("ID");
		result.types.push_back("INT64");

		try {
			int64_t insertedTo = table.insert(query.values, info->columns);
			result.messeage = std::format("Inserted into {}", insertedTo);
			result.body.push_back({ insertedTo });

			result.isSucces = true;
		}
		catch (std::runtime_error& error) {
			result.error = std::move(error.what());
		}
		catch (std::exception& error) {
			result.error = std::move(error.what());
		}

	}

	void executeDelete(DeleteQuery& query, Result& result) {

		auto& table = *query.table_ptr;
		auto& storage = table.getStoarge();
		auto info = table.getInfo();
		auto& columns = info->columns;
		auto& conditions = query.conditions;
		
		result.header.push_back("ID");
		result.types.push_back("INT64");

		//  DIRECT INDEX - O(1) оптимизация
		if (!conditions.empty() && conditions[0].cond_type == ConditionType::DIRECT_INDEX) {
			int64_t line_id = std::get<int64_t>(conditions[0].desired);
			if (storage.isActive(line_id)) {
				table.erase(line_id);
				result.isSucces = true;
			}
			result.body.push_back({ line_id });
			return;
		}

		//  FULL SCAN с фильтрацией
		size_t max_line = table.getMaxActiveLine();

		size_t active_count = table.getActivePlaces();
		active_count = active_count < query.limit ? active_count : query.limit;

		result.body.reserve(active_count);

		for (int64_t i = 0; i < max_line && result.body.size() < active_count; ++i) {
			if (!storage.isActive(i)) continue;
			if (!conditions.empty() and !satisfiesConditions(table, i, conditions, columns)) continue;

			table.erase(i);
			result.body.push_back({ i });
		}
		result.isSucces = true;

	}

	void executeUpdate(UpdateQuery& query, Result& result) {

		auto& table = *query.table_ptr;
		auto& storage = table.getStoarge();
		auto info = table.getInfo();
		auto& columns = info->columns;
		auto& conditions = query.conditions;
		auto& columns_optimized = query.columns_optimized;

		result.header.push_back("ID");
		result.types.push_back("INT64");

		std::vector<Column> columns_to_update;
		columns_to_update.reserve(columns_optimized.size());
		for (size_t i : columns_optimized) {
			columns_to_update.push_back(columns[i]);
		}

		//  DIRECT INDEX - O(1) оптимизация
		if (!conditions.empty() and conditions[0].cond_type == ConditionType::DIRECT_INDEX) {
			int64_t line_id = std::get<int64_t>(conditions[0].desired);
			if (storage.isActive(line_id)) {
				table.update(query.values, columns_to_update, line_id);
				result.body.push_back({ line_id });
				result.isSucces = true;
			}
			return;
		}

		//  FULL SCAN с фильтрацией
		size_t max_line = table.getMaxActiveLine();

		size_t active_count = table.getActivePlaces();
		active_count = active_count < query.limit ? active_count : query.limit;

		result.body.reserve(active_count);

		for (int64_t i = 0; i < max_line && result.body.size() < active_count; ++i) {
			if (!storage.isActive(i)) continue;
			if (!conditions.empty() and !satisfiesConditions(table, i, conditions, columns)) continue;

			table.update(query.values, columns_to_update, i);

			result.body.push_back({i});
		}
		

	}

public:



	void execute(QueryVariant& query, Result& res) {

		res.isSucces = true;

		std::visit([&res, this](auto& query) {
			using T = std::decay_t<decltype(query)>;
			
			if constexpr (std::is_same_v<T, SelectQuery>) {
				executeSelect(query, res);
			}
			if constexpr (std::is_same_v<T, InsertQuery>) {
				executeInsert(query, res);
			}
			if constexpr (std::is_same_v<T, DeleteQuery>) {
				executeDelete(query, res);
			}
			if constexpr (std::is_same_v<T, UpdateQuery>) {
				executeUpdate(query, res);
			}

			}, query);


	}

};