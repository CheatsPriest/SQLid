#pragma once

#include <vector>
#include <string>
#include "Conditions.h"
#include <memory>

enum class QueryType { SELECT, UPDATE, INSERT, ERASE };

class Query {
public:
    QueryType type;// Для SELECT, UPDATE
    std::string table_name;

    std::vector<std::string> columns_raw; 
    std::vector<size_t> columns_optimized;

    std::vector<Condition> conditions; // Условие WHERE
    std::vector<std::string> raw_conditions;

    size_t limit = SIZE_MAX;                // LIMIT


    

    ~Query() = default;
};