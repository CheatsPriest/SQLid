#pragma once
#include "Query.h"
#include <string>
#include <sstream>

class RequestParser {

	void parseConditionAndLimit(Query& result, std::istringstream& str) {

		std::string buf;
		str >> buf;
		if (buf == "WHERE") {
			while (str >> buf) {
				if (buf == "LIMIT")break;
				result.raw_conditions.push_back(std::move(buf));
			}
		}
		if (buf == "LIMIT") {
			str >> result.limit;
		}

	}

public:
	RequestParser() {};

	Query parse(const std::string& command) {

		Query result;

		std::istringstream str(command);

		std::string queryType, buf;
		str >> queryType;
		if (queryType == "SELECT") {
			result.type = QueryType::SELECT;

			
			while (true) {
				str >> buf;
				if (buf == "FROM")break;
				result.columns_raw.push_back(std::move(buf));
			}
			str >> buf;
			result.table_name = std::move(buf);

			parseConditionAndLimit(result, str);

		}
		else if (queryType == "UPDATE") {
			result.type = QueryType::UPDATE;


		}
		else if (queryType == "DELETE") {
			result.type = QueryType::ERASE;

		}
		else if (queryType == "INSERT") {
			result.type = QueryType::INSERT;

		}

		return result;
	}

};