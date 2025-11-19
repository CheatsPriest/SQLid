#pragma once
#include "parser/CommandsParser.h"
#include "result/Result.h"
#include "clients/Client.h"
#include "system/system.h"

class CommandsExecutor {
private:
	
public:
	void execute(CommandsVariant& command, Result& res, ClientInfo& clientInfo, System& sys) {

		

		std::visit([&res, &clientInfo, &sys, this](auto& command) {
			using T = std::decay_t<decltype(command)>;

			if constexpr (std::is_same_v<T, AttachCommand>) {
				try {
					clientInfo.baseId = sys.getDataBaseId(command.databaseName);
					clientInfo.joinedToDataBase = true;
					res.messeage = "Succes";
				}
				catch (...) {
					throw IncorrectInputException("Unknown database: " + command.databaseName);
				}
			}
			if constexpr (std::is_same_v<T, CreateDatabaseCommand>) {
				sys.createDataBase(command.databaseName);
				res.messeage = "Database created";
			}
			if constexpr (std::is_same_v<T, CreateTableForDatabaseCommand>) {
				sys.createTable(command.stream);
				res.messeage = "Table created";
			}
			

			}, command);

		res.isSucces = true;
	}
};