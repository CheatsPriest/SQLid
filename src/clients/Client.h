#pragma once
#include "Base.h"



class ClientInfo {
private:


public:
	size_t id;
	bool isConsole;
	bool joinedToDataBase;
	size_t baseId;
	std::unique_ptr<BaseConnection> connection;
	ClientInfo(size_t id_, std::unique_ptr<BaseConnection>&& connection_) :
		id(id_), connection(std::move(connection_)), isConsole(false), joinedToDataBase(false), baseId(0) {

	}
};

class Client {
private:
	boost::asio::io_context& io_context;
	std::unique_ptr<BaseConnection> connection_;

	tcp::resolver resolver;
	std::string name, service;



public:
	void establishCommunucation() {

		try {
			auto endpoints = resolver.resolve(name, service);
			boost::asio::connect(connection_->socket(), endpoints);
		}
		catch (const std::exception& e) {
			std::cout << "Connection failed: " << e.what() << std::endl;
			throw;  
		}

	}
	BaseConnection& getConnection() {
		return *connection_;
	}
	
	void connect() {
		auto endpoints = resolver.resolve(name, service);
		boost::asio::connect(connection_->socket(), endpoints);
	}

	bool is_connected() const {
		return connection_->socket().is_open();
	}

	void disconnect() {
		boost::system::error_code ec;
		connection_->socket().close(ec);
	}

	Client(boost::asio::io_context& io_context_, std::string_view name_, std::string_view service_) : io_context(io_context_), name(name_),
		service(service_), connection_(std::make_unique< BaseConnection>(io_context_)), resolver(io_context_) {


	}

};