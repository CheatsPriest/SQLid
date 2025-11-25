#pragma once

#include <string>
#include <variant>



template<typename Derived>
class CommandBase {
public:


};

class AttachCommand : public CommandBase<AttachCommand> {
public:
	std::string databaseName;
	AttachCommand(std::string_view databaseName_) :databaseName(databaseName_) {

	}
};

class CreateDatabaseCommand : public CommandBase<CreateDatabaseCommand> {
public:
	std::string databaseName;
	CreateDatabaseCommand(std::string_view databaseName_) :databaseName(databaseName_) {

	}
};
class CreateTableForDatabaseCommand : public CommandBase<CreateTableForDatabaseCommand> {
public:
	std::stringstream& stream;
	CreateTableForDatabaseCommand(std::stringstream& stream_) : stream(stream_) {

	}
};

class BackupDatabase : public CommandBase<BackupDatabase> {
public:
	const std::string base, backup;
	BackupDatabase(const std::string& base_, const std::string& backup_) : base(base_), backup(backup_) {

	}
};
class RestoreDatabase : public CommandBase<RestoreDatabase> {
public:
	const std::string base, backup;
	RestoreDatabase(const std::string& backup_, const std::string& base_) : base(base_), backup(backup_) {

	}
};
class ShowStructure : public CommandBase<ShowStructure> {
public:
	
	ShowStructure(){

	}
};
class DropDatabase : public CommandBase<DropDatabase> {
public:
	const std::string base;
	DropDatabase(const std::string& base_) : base(base_) {

	}
};
class HelpCommand : public CommandBase<HelpCommand> {
public:
	HelpCommand() {

	}
};

using CommandsVariant = std::variant<
	AttachCommand,
	CreateDatabaseCommand,
	CreateTableForDatabaseCommand,
	BackupDatabase,
	RestoreDatabase,
	ShowStructure,
	DropDatabase,
	HelpCommand
>;