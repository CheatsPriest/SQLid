#pragma once
#include <variant>
#include <string>

using variant_types = std::variant<int32_t, int64_t, double, float, bool, std::string>;
enum class Type { VOIDA = 0, INT32 = 1, INT64 = 2, DOUBLE = 3, FLOAT = 4, BOOL = 5, TEXT = 6, STRING = 7 };

static constexpr std::array<size_t, 8> type_sizes = {
	0,  // VOIDA
	sizeof(int),  // INT32
	sizeof(long long),  // INT64  
	sizeof(double),  // DOUBLE
	sizeof(float),  // FLOAT
	sizeof(bool),  // BOOL
	128,  // TEXT 
	0   // STRING - User decides on his own size
};



static variant_types parse_value(const std::string& str, Type type) {


	switch (type) {
	case Type::INT32: return variant_types{ static_cast<int32_t>(std::stoi(str)) };
	case Type::INT64: return variant_types{ static_cast<int64_t>(std::stoll(str)) };
	case Type::DOUBLE: return variant_types{ static_cast<double>(std::stod(str)) };
	case Type::FLOAT: return variant_types{ static_cast<float>(std::stof(str)) };
	case Type::BOOL:
		if (str == "true" or str == "1") return variant_types{ true };
		if (str == "false" or str == "0") return variant_types{ false };
		throw std::runtime_error("Invalid boolean value: " + str);
	case Type::STRING:
	case Type::TEXT:
		return variant_types{ str }; 
	default:
		throw std::runtime_error("Unsupported type");
	}
}