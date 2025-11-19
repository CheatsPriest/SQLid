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