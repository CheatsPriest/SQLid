#pragma once
#include "parser/CommandsParser.h"
#include "result/Result.h"
#include "clients/Client.h"
#include "system/system.h"
#include "parser/Help.h"

class CommandsExecutor {
private:
	

	void showStructure(Result& res, System& sys) {
		std::shared_lock<std::shared_mutex> lock(sys.getMtx());
		for (auto& base : sys.getBases()) {
			if(base.get()!=nullptr)
				res.body.push_back({ base->getBaseNameAndTablesNames() });
		}
	}

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
			if constexpr (std::is_same_v<T, BackupDatabase>) {
				sys.backUpDatabase(command.base, command.backup);
				res.messeage = "Backuped";
			}
			if constexpr (std::is_same_v<T, RestoreDatabase>) {
				sys.restoreDatabase(command.backup, command.base);
				res.messeage = "Restored";
			}
			if constexpr (std::is_same_v<T, ShowStructure>) {
				
				showStructure(res, sys);
				res.messeage = "Info:";
			}
			if constexpr (std::is_same_v<T, DropDatabase>) {
				sys.deleteDatabase(command.base);
				res.messeage = "Base dropped";
			}
			if constexpr (std::is_same_v<T, HelpCommand>) {
				res.body = std::move(getHelp());
				res.messeage = "Some info ...";
			}

			}, command);

		res.isSucces = true;
	}
};