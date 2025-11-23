#pragma once
#include <string>
#include <sstream>
#include "exceptions/Exceptions.h"
#include "Commands.h"

// CREATE DATABASE TestDataBase
// CREATE TABLE FOR TestDataBase TestTable result BOOL time INT64 name STRING[10] velocity FLOAT month TEXT
// ATTACH TO TestDataBase
// BACKUP basename AS tackupname
// RESTORE backupname AS basename
class CommandsParser {
private:
	
public:
	CommandsVariant parse(std::stringstream& stream) {
		std::string buf;
		
		stream >> buf;
		if (buf == "CREATE") {
			stream >> buf;
			if (buf == "TABLE") {
				stream >> buf;
				if(buf=="FOR")
					return CreateTableForDatabaseCommand(stream);
			}
			else if(buf == "DATABASE") {
				stream >> buf;
				return CreateDatabaseCommand(buf);
			}
		}
		else if (buf == "ATTACH") {
			stream >> buf;
			if (buf == "TO") {
				stream >> buf;
				return AttachCommand(buf);
			}
		}
		else if (buf == "BACKUP") {
			std::string name1, name2;
			
			stream >> name1;
			stream >> buf;
			if (buf == "AS") {
				stream >> name2;
				return BackupDatabase(name1, name2);
			}
		}
		else if (buf == "RESTORE") {
			std::string name1, name2;

			stream >> name1;
			stream >> buf;
			if (buf == "AS") {
				stream >> name2;
				return RestoreDatabase(name1, name2);
			}
		}
		else if (buf == "SHOW") {
			return ShowStructure();
		}
		else if (buf == "DROP") {
			stream >> buf;
			if (buf == "DATABASE" or buf=="BASE") {
				stream >> buf;
				return DropDatabase(buf);
			}
		}
		throw IncorrectInputException("Unknown command: " + buf);
	}
};