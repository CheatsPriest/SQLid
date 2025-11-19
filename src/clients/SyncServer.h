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
	std::jthread acceptor_thread;

public:
	SyncServer(boost::asio::io_context& io_context__, int port) : 
		io_context_(io_context__), acceptor_(io_context__, tcp::endpoint(tcp::v4(), port)), quite{false}
	{
		acceptor_thread = std::jthread([this](std::stop_token st) {
			accept_loop(st);
			});
	}

	void start_accept()
	{
		
		auto new_connection = std::make_unique<BaseConnection>(io_context_);
		acceptor_.accept(new_connection->socket());
		addNewClient(std::move(new_connection));
		
	}
	void accept_loop(std::stop_token st) {
		while (!st.stop_requested()) {
			auto new_connection = std::make_unique<BaseConnection>(io_context_);

			try {
				acceptor_.accept(new_connection->socket());

				if (!st.stop_requested()) {
					addNewClient(std::move(new_connection));
				}
			}
			catch (const boost::system::system_error& e) {
				if (e.code() != boost::asio::error::operation_aborted) {
					std::cout << "Accept error: " << e.what() << std::endl;
				}
				break;
			}
		}
	}
	void shutdown() {
		for (auto& thr : threads) {
			thr.second.request_stop();
		}
	}
	void shutdownAll() {
		std::unique_lock<std::shared_mutex> lock(add_mtx);

		quite.store(true);

		// 0. Останавливаем acceptor thread
		acceptor_thread.request_stop();

		boost::system::error_code ec;
		acceptor_.close(ec);

		// 1. Отправляем DISCONNECT всем клиентам
		for (auto& [client_id, client_info] : clients) {
			try {
				client_info.connection->sendText("DISCONNECT");
			}
			catch (...) {
				// Игнорируем ошибки
			}
		}

		// 2. Останавливаем все потоки
		for (auto& [client_id, jthread] : threads) {
			jthread.request_stop();
		}

		// 3. Закрываем все сокеты
		for (auto& [client_id, client_info] : clients) {
			boost::system::error_code ec;
			client_info.connection->socket().close(ec);
		}

		// 4. Очищаем структуры
		clients.clear();
		threads.clear();


		std::cout << "Server completely shutdown." << std::endl;
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

			if (buf == "DISCONNECT") {
				Result request;
				request.isSucces = true;
				request.messeage = "DISCONNECTED";
				err = connection.send_json(request.to_json());
				break;
			}
			if (buf == "LEAVE") {
				Result request;
				request.isSucces = true;
				request.messeage = "LEAVED";
				err = connection.send_json(request.to_json());
				client.joinedToDataBase = false;
				continue;
			}

			if (client.joinedToDataBase) {
				auto request = oper.execute(client, buf);
				//request.print();

				err = connection.send_json(request.to_json());
				if (err)break;
			}
			else {
				auto request = oper.executeCommand(client, buf);
				//request.print();

				err = connection.send_json(request.to_json());
				if (err)break;
			}
			

		}
	}
};