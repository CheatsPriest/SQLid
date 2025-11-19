#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>

using boost::asio::ip::tcp;


class BaseConnection : public std::enable_shared_from_this<BaseConnection>
{
public:


	static std::shared_ptr<BaseConnection> create(boost::asio::io_context& io_context)
	{
		return std::shared_ptr<BaseConnection>(new BaseConnection(io_context));
	}

	tcp::socket& socket()
	{
		return socket_;
	}


	boost::system::error_code send_json(const boost::json::value& json) {
		std::string json_str = boost::json::serialize(json);
		size_t size = json_str.size();

		boost::system::error_code ec;

		boost::asio::write(socket_, boost::asio::buffer(&size, sizeof(size)), ec);
		if (ec)return ec;

		boost::asio::write(socket_, boost::asio::buffer(json_str), ec);

		return ec;
	}


	boost::system::error_code read_json(boost::json::value& json) {
		boost::system::error_code ec;
		size_t size;
		boost::asio::read(socket_, boost::asio::buffer(&size, sizeof(size)));
		if (ec) return ec;

		std::vector<char> buffer(size);
		boost::asio::read(socket_, boost::asio::buffer(buffer), ec);
		if (ec) return ec;

		json = boost::json::parse(std::string(buffer.begin(), buffer.end()), ec);
		return ec;
	}

	boost::system::error_code readText(std::string& in) {
		boost::system::error_code error;
		boost::asio::read(socket_, boost::asio::buffer(in), error);
		return error;
	}


	boost::system::error_code sendText(std::string_view message) {
		boost::system::error_code error;
		boost::asio::write(socket_, boost::asio::buffer(message), error);
		return error;
	}

	template<typename T>
	boost::system::error_code readNumber(T& in) {
		boost::system::error_code error;
		size_t len = socket_.read_some(boost::asio::buffer(&in, sizeof(T)), error);
		return error;
	}

	template<typename T>
	boost::system::error_code sendNumber(T&& message) {
		boost::system::error_code error;
		socket_.write_some(boost::asio::buffer(&message, sizeof(T)), error);
		return error;
	}
	BaseConnection(boost::asio::io_context& io_context)
		: socket_(io_context)
	{
	}



private:


	tcp::socket socket_;

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
// NOT THREAD SAFE. 1 CLIENT = 1 CONNENCTION. 
class SQLid_API {
private:
	boost::asio::io_context& io_context_;
	Client client;
	std::string ip, service;
	boost::system::error_code err;
	
public:

	SQLid_API(boost::asio::io_context& io_context__, std::string_view ip_, std::string_view service_) : 
		io_context_(io_context__), ip(ip_), service(service_), client(io_context__, ip, service_) {

	}
	void connect() {
		client.connect();
	}
	void sendText(std::string_view text) {
		auto& connection = client.getConnection();


		err= connection.sendNumber(text.size());
		if (err)throw boost::system::error_code(err);
		err=connection.sendText(text);
		if (err)throw boost::system::error_code(err);

	}
	std::string readText() {
		auto& connection = client.getConnection();
		size_t size = 0;
		std::string in;

		err= connection.readNumber(size);
		if (err)throw boost::system::error_code(err);
		in.resize(size, '\0');
		err= connection.readText(in);
		if (err)throw boost::system::error_code(err);
		return in;
	}
	void sendJson(const boost::json::value& json) {
		err= client.getConnection().send_json(json);
		if (err)throw boost::system::error_code(err);
	}
	boost::json::value readJson() {
		boost::json::value json;
		err=client.getConnection().read_json(json);
		if (err)throw boost::system::error_code(err);
		return json;
	}

	std::string joinToDataBase(std::string_view name) {
		sendText(name);
		return readText();
	}
	boost::json::value request(std::string_view req) {
		sendText(req);
		return readJson();
	}
	~SQLid_API() {
		try {
			sendText("DISCONNECT");
		}
		catch (...) {

		}
	}
};


