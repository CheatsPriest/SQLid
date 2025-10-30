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