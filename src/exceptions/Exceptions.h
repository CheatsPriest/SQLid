#pragma once
#include <exception>
#include <string>

class IncorrectInputException : public std::exception {
private:
    std::string message;

public:
    
    explicit IncorrectInputException(const std::string& msg)
        : message("Incorret input: " + msg) {
    }

    
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class ExecutionException : public std::exception {
private:
    std::string message;

public:
    
    explicit ExecutionException(const std::string& msg)
        : message("Incorret input: " + msg) {
    }

    
    const char* what() const noexcept override {
        return message.c_str();
    }
};