#pragma once
#include "Query.h"
#include <string>
#include <sstream>

class IncorrectInput : public std::exception {

public:

	const char* what() const noexcept override {
		return "Incorrect input";
	}
};

class RequestParser {

	void parseConditionAndLimit(SelectQuery& result, std::istringstream& str) {

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

	SelectQuery parseSelect(std::istringstream& str) {
		SelectQuery result; 
		std::string buf;

		while (str >> buf) {
			if (buf == "FROM")break;
			result.columns_raw.push_back(std::move(buf));
		}
		if (buf != "FROM")throw IncorrectInput();

		str >> buf;
		result.tabble_name = std::move(buf);

		parseConditionAndLimit(result, str);

		return result;
	}

	InsertQuery parseInsert(std::istringstream& str) {
		InsertQuery result;
		std::string buf;
		str >> buf;
		if (buf != "INTO")throw IncorrectInput();

		str >> result.tabble_name;

		while (str >> buf) {
			result.raw_values.push_back(std::move(buf));
		}

		return result;
	}

public:
	RequestParser() {};

	QueryVariant parse(const std::string& command) {

		

		std::istringstream str(command);

		std::string queryType;
		str >> queryType;
		if (queryType == "SELECT") return parseSelect(str);
		else if (queryType == "INSERT") return parseInsert(str);
		else if (queryType == "DELETE") {
			//result.type = QueryType::ERASE;

		}
		else if (queryType == "UPDATE") {
			//result.type = QueryType::INSERT;

		}

		throw std::runtime_error("Unsupported command");
	}

};