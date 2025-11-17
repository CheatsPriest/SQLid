#pragma once

#include "system/system.h"
#include "data_base/Executor.h"
#include "parser/RequestParser.h"
#include "clients/Client.h"

#include <string>

class Operator {
private:
	std::string path = "C:\\TestDataBase";

public:
	System sys;
	Executor exec;
	RequestParser parser;
	Operator() : sys(path) {
		
	}
	Result execute(const ClientInfo& client, const std::string command) {
		Result result;
		DataBase& base = sys.getDataBaseById(client.baseId);

		try {

			auto query = parser.parse(command);
			base.optimizing(query);
			exec.execute(query, result);

		}
		catch (IncorrectInputException& error) {
			result.error = error.what();
			result.isSucces = false;
			return result;
		}
		catch (ExecutionException& error) {
			result.error = error.what();
			result.isSucces = false;
			return result;
		}
		
		
		
		result.isSucces = true;
		return result;
	}

};