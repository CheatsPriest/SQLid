#pragma once
#include "Query.h"
#include <string>
#include <sstream>
#include "exceptions/Exceptions.h"


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
		if (buf != "FROM")throw IncorrectInputException("Missed keyworld \"FROM\"");

		str >> buf;
		result.tabble_name = std::move(buf);

		parseConditionAndLimit(result, str);

		return result;
	}

	InsertQuery parseInsert(std::istringstream& str) {
		InsertQuery result;
		std::string buf;
		str >> buf;
		if (buf != "INTO")throw IncorrectInputException("Missed keyworld \"INTO\"");

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
		if (buf != "FROM")throw IncorrectInputException("Missed keyworld \"FROM\"");

		str >> result.tabble_name;
		
		

		parseConditionAndLimit(result, str);

		return result;
	}

	UpdateQuery parseUpdate(std::istringstream& str) {
		UpdateQuery result;
		std::string buf;
		str >> result.tabble_name;
		str >> buf;
		if (buf != "SET") throw IncorrectInputException("Missed keyworld \"SET\"");

		while (str >> buf) {
			if (buf == "WHERE" or buf == "LIMIT")break;
			result.columns_raw.push_back(std::move(buf));
			str >> buf;
			if(buf!="=")throw IncorrectInputException("Missed symbol \'=\'");
			str >> buf;
			result.raw_values.push_back(std::move(buf));
		}


		while (buf!="LIMIT" and str >> buf) {
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

		throw IncorrectInputException("Unsupported command");
	}

};