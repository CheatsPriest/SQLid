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

	template<typename Query>
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

	DeleteQuery parseDelete(std::istringstream& str) {
		DeleteQuery result;
		std::string buf;
		str >> buf;
		if (buf != "FROM")throw IncorrectInput();

		str >> result.tabble_name;
		
		

		parseConditionAndLimit(result, str);

		return result;
	}

	UpdateQuery parseUpdate(std::istringstream& str) {
		UpdateQuery result;
		std::string buf;
		str >> result.tabble_name;
		str >> buf;
		if (buf != "SET") throw IncorrectInput();

		while (str >> buf) {
			if (buf == "WHERE")break;
			result.columns_raw.push_back(std::move(buf));
			str >> buf;
			if(buf!="=")throw IncorrectInput();
			str >> buf;
			result.raw_values.push_back(std::move(buf));
		}


		while (str >> buf) {
			if (buf == "LIMIT")break;
			result.raw_conditions.push_back(std::move(buf));
		}

		if (buf == "LIMIT") {
			str >> result.limit;
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
		else if (queryType == "DELETE") return parseDelete(str);
		else if (queryType == "UPDATE") return parseUpdate(str);

		throw std::runtime_error("Unsupported command");
	}

};