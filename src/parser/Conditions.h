#pragma once
#include <array>
#include <functional>
#include "supported_variants.h"

enum class ConditionType { EQUAL=0, NOT_EQUAL=1, GREATER=2, LESS=3, AND=4, OR=5, LOGICAL_AND, LOGICAL_OR };

static std::unordered_map<std::string, ConditionType> condition_map{
    {"==", ConditionType::EQUAL},
    {"!=", ConditionType::NOT_EQUAL},
    {">", ConditionType::GREATER},
    {"<", ConditionType::LESS},
    {"AND", ConditionType::AND},
    {"OR", ConditionType::OR},
};

static std::array<std::function<bool(variant_types, variant_types)>, 6> functions{
    [](variant_types a, variant_types b) -> bool { return a == b; },  // EQUAL
    [](variant_types a, variant_types b) -> bool { return a != b; },   // NOT_EQUAL
    [](variant_types a, variant_types b) -> bool { return a > b; },   // GREATER
    [](variant_types a, variant_types b) -> bool { return a < b; },   // LESS
    [](variant_types a, variant_types b) -> bool { return std::get<bool>(a) and std::get<bool>(b); },   // AND
    [](variant_types a, variant_types b) -> bool { return std::get<bool>(a) or std::get<bool>(b);  },   // OR
};

class Condition {

private:
    
public:
    ConditionType cond_type;
    size_t collumn_id;

    variant_types desired;

    Condition(ConditionType cond_type_, size_t collumn_id_, variant_types desired_) : cond_type(cond_type_), collumn_id(collumn_id_),
        desired(desired_){}

    bool result(variant_types value) {
        return functions[(size_t)cond_type](value, desired);
    }

};



