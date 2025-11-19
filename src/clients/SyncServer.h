#pragma once

#include <unordered_map>
#include <thread>
#include <array>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <iostream>

#include "Base.h"
#include "clients/Operator.h"
#include "clients/Client.h"







class SyncServer {
private:
	// DATA BASE OPERATOR
	Operator oper;
	
	// Connections data
	std::shared_mutex add_mtx;
	size_t freeId = 0;
	std::unordered_map<size_t, ClientInfo> clients;
	std::unordered_map<size_t, std::jthread> threads;

	// Server data
	boost::asio::io_context& io_context_;
	tcp::acceptor acceptor_;
	std::atomic<bool> quite;
public:
	SyncServer(boost::asio::io_context& io_context__, int port) : 
		io_context_(io_context__), acceptor_(io_context__, tcp::endpoint(tcp::v4(), port)), quite{false}
	{
		
	}

	void start_accept()
	{
		auto new_connection = std::make_unique<BaseConnection>(io_context_);
		acceptor_.accept(new_connection->socket());
		addNewClient(std::move(new_connection));
	}
	void shutdown() {
		for (auto& thr : threads) {
			thr.second.request_stop();
		}
	}
private:
	void addNewClient(std::unique_ptr<BaseConnection>&& connection) {
		std::unique_lock<std::shared_mutex> lock(add_mtx);
		size_t curId = freeId++;
		clients.insert({ curId, {curId, std::move(connection)} });
		threads.insert({ curId, std::jthread(std::bind(&SyncServer::processQueries, this, std::placeholders::_1, curId))});
	}

	void processQueries(std::stop_token st, size_t id){

		std::cout << "started connection"<<id << std::endl;

		std::shared_lock<std::shared_mutex> lock(add_mtx);
		ClientInfo& client = clients.at(id);
		BaseConnection& connection = *client.connection;
		lock.unlock();
		std::cout << "started connection" << std::endl;
		boost::json::value js;
		size_t num = 0;
		std::string buf;
		boost::system::error_code err;

		while (!st.stop_requested()) {
			buf = "";
			std::cout << "Work" << std::endl;

			connection.readNumber(num);
			if (err)break;

			buf.resize(num, '\0');

			err = connection.readText(buf);
			if (err)break;

			if (buf == "DISCONNECT")break;
			if (buf == "LEAVE") {
				client.joinedToDataBase = false;
				continue;
			}

			if (client.joinedToDataBase) {
				auto request = oper.execute(client, buf);
				request.print();

				err = connection.send_json(request.to_json());
				if (err)break;
			}
			else {
				auto request = oper.executeCommand(client, buf);
				request.print();

				err = connection.send_json(request.to_json());
				if (err)break;
			}
			
			/*if (client.joinedToDataBase) {

				
				
				if (err)break;

				if (buf == "END")break;
				if (buf == "LEAVE") {
					client.joinedToDataBase = false;
					Result res;
					res.messeage = "Detached from database";
					err = connection.send_json(res.to_json());
					if (err)break;

					continue;
				}
				auto request = oper.execute(client, buf);
				request.print();
				
				err = connection.send_json(request.to_json());
				if (err)break;

				std::cout << "Json send" << std::endl;
				
				
			}
			else {
				err=connection.readNumber(num);
				if (err)break;

				buf.resize(num, '\0');
				err = connection.readText(buf);
				if (err)break;
				if (buf == "END")break;

				std::cout << buf << std::endl;
				if (buf == "0" or buf == "EXIT")break;
				
				try {
					size_t baseId = oper.sys.getDataBaseId(buf);
					std::cout << baseId << std::endl;
					client.baseId = baseId;
					client.joinedToDataBase = true;
					Result reply;
					reply.isSucces = true;
					reply.messeage = "Succes";
					connection.send_json(reply.to_json());
				}
				catch (...) {
					Result reply;
					reply.isSucces = false;
					reply.error = "Unknown data base: " + buf;
					connection.send_json(reply.to_json());
					
					if (err)break;
				}
				
			}*/


		}

		

	}
};