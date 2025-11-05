#pragma once
#include <iostream>
#include <vector>
#include "supported_variants.h"
#include <iomanip>

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

};