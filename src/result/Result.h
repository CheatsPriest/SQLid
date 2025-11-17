#pragma once
#include <iostream>
#include <vector>
#include "supported_variants.h"
#include <iomanip>
#include <boost/json.hpp>

class Result {
private:


public:
	bool isSucces;
	std::vector<std::string> header;
	std::vector<std::string> types;
	std::string messeage;
	std::string error;
	std::vector<std::vector<variant_types>> body;
	Result() : isSucces(false) {};


    void print() const {
        if (!isSucces) {
            std::cout << "Error: " << error << std::endl;
            return;
        }

        if (!messeage.empty()) {
            std::cout << "Message: " << messeage << std::endl;
        }

        // Вывод заголовков
        if (!header.empty()) {
            std::cout << "| ";
            for (const auto& col : header) {
                std::cout << std::setw(15) << col << " | ";
            }
            std::cout << std::endl;

            // Разделитель
            std::cout << "+";
            for (size_t i = 0; i < header.size(); ++i) {
                std::cout << std::string(17, '-') << "+";
            }
            std::cout << std::endl;
        }
        if (!types.empty()) {
            std::cout << "| ";
            for (const auto& col : types) {
                std::cout << std::setw(15) << col << " | ";
            }
            std::cout << std::endl;

            // Разделитель
            std::cout << "+";
            for (size_t i = 0; i < types.size(); ++i) {
                std::cout << std::string(17, '-') << "+";
            }
            std::cout << std::endl;
        }

        // Вывод данных
        for (const auto& row : body) {
            std::cout << "| ";
            for (const auto& cell : row) {
                std::visit([](const auto& value) {
                    std::cout << std::setw(15) << value << " | ";
                    }, cell);
            }
            std::cout << std::endl;
        }

        std::cout << "Total rows: " << body.size() << std::endl;
    }

    boost::json::value to_json() {  
        boost::json::object result;

        result["success"] = isSucces;

        if (!isSucces) {
            result["error"] = std::move(error);  
            return result;
        }

        if (!messeage.empty()) {
            result["message"] = std::move(messeage); 
        }

        // Заголовки и типы
        if (!header.empty()) {
            result["headers"] = boost::json::value_from(std::move(header));  
        }

        if (!types.empty()) {
            result["types"] = boost::json::value_from(std::move(types));  
        }

        // Тело данных - преобразуем variant_types в JSON
        if (!body.empty()) {
            boost::json::array json_body;
            json_body.reserve(body.size());  

            for (auto& row : body) {
                boost::json::array json_row;
                json_row.reserve(row.size());

                for (auto& cell : row) {
                    json_row.push_back(boost::json::value_from(cell));  
                }
                json_body.push_back(std::move(json_row));
            }
            result["data"] = std::move(json_body);  
        }

        result["total_rows"] = body.size();

        return result;
    }

};