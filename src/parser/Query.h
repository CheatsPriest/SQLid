#pragma once


#include <vector>
#include <string>
#include "Conditions.h"
#include <memory>
#include "files/InfoLoader.h"
#include "files/Tabble.h"
#include "result/Result.h"
#include <format>


enum class QueryType { SELECT, UPDATE, INSERT, ERASE };



template<typename Derived>
class QueryBase {
public:
    std::string tabble_name;
	Tabble* table_ptr=nullptr;
    std::vector<Condition> conditions;
    std::vector<std::string> raw_conditions;
    size_t limit = SIZE_MAX;

    
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

};


struct oderTypeRaw {
    std::string rawColumn;
    bool isGreaterSort;
};
struct oderTypeOpt {
    size_t rawId;
    bool isGreaterSort;
};
class SelectQuery : public QueryBase<SelectQuery> {


private:


public:
    std::vector<std::string> columns_raw;
    std::vector<size_t> columns_optimized;

    bool needToSort;
    std::vector<oderTypeRaw> oder_columns_raw;
    std::vector<oderTypeOpt> order_columns_optimized;
	
};


class InsertQuery : public QueryBase<InsertQuery> {
public:
    std::vector<std::string> raw_values;
    std::vector<variant_types> values;

};


class DeleteQuery : public QueryBase<DeleteQuery>{
public:

};

class UpdateQuery : public QueryBase< UpdateQuery> {
public:

    std::vector<std::string> columns_raw;
    std::vector<size_t> columns_optimized;

    std::vector<std::string> raw_values;
    std::vector<variant_types> values;

};

using QueryVariant = std::variant<
    SelectQuery,
    UpdateQuery,
    InsertQuery,
    DeleteQuery
>;