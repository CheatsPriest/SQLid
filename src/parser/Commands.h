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

using CommandsVariant = std::variant<
	AttachCommand,
	CreateDatabaseCommand,
	CreateTableForDatabaseCommand
>;