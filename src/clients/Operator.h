#pragma once

#include "system/system.h"
#include "data_base/Executor.h"
#include "parser/RequestParser.h"
#include "clients/Client.h"

#include "data_base/CommandsExecutor.h"

#include <string>

class Operator {
private:
	std::string path = "C:/TestDataBase";
	//std::string path = "testBase";

public:
	System sys;

	Executor exec;
	CommandsExecutor comExex;

	RequestParser parser;
	CommandsParser comParser;

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
	Result executeCommand(ClientInfo& client, std::string& command) {
		Result result;
		std::cout << "started" << std::endl;
		
		std::cout << "geted" << std::endl;
		std::stringstream stream(std::move(command));
		std::cout << "streamed" << std::endl;
		
		try {
			auto com = comParser.parse(stream);
			comExex.execute(com, result, client, sys);
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

	void createDataBase(const std::string& name) {
		sys.createDataBase(name);
	}
	void createTableInDataBase(std::stringstream& stream) {
		sys.createTable(stream);
	}
};