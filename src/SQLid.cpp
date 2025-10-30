// SQLid.cpp: определяет точку входа для приложения.
//

#include "SQLid.h"
#include <thread>
#include "files/InfoLoader.h"
#include "parser/Conditions.h"
#include "parser/RequestParser.h"
#include "data_base/DataBase.h"

int main()
{



	DataBase base;
	base.openTabble("C:\\TestDataBase\\test", "test");

	RequestParser parser;
	auto result = parser.parse("SELECT age FROM test WHERE age == 10 AND age == 34 OR name == ""BABA"" LIMIT 100");

	base.optimizeQuery(result);

	std::cout << result.table_name << std::endl;
	for (auto& el : result.columns_raw) {
		std::cout << "NAME " << el << std::endl;
	}
	for (auto& el : result.raw_conditions) {
		std::cout << "CONDOTION " << el << std::endl;
	}
	std::cout << result.limit << std::endl;

	

	/*
	

	std::cout << result.table_name << std::endl;
	for (auto& el : result.columns_raw) {
		std::cout <<"NAME "<< el << std::endl;
	}
	for (auto& el : result.raw_conditions) {
		std::cout << "CONDOTION "<<el << std::endl;
	}
	std::cout << result.limit << std::endl;*/

	/*std::string files = "C:\\TestDataBase\\test";

	Tabble base1(files);

	Condition cond(ConditionType::EQUAL, 0, 10);
	std::cout << cond.result(10) << std::endl;*/

	/*MemoryMap map("test.db", 12);

	map.writeText(10, 0, "123456789ab");
	auto str  = map.readText(10, 0, 8);
	map.writeText(123, 0, "123456789ab");
	std::cout << str << std::endl;*/
	/*std::vector<std::jthread> pool;
	pool.reserve(6);

	for (int i = 0; i < 100; ++i) {
		map.writeNumber<int>(i, 0, i);
		map.writeNumber<long long>(i, 4, i*100);
	}

	for (int i = 0; i < 6; i++) {
		pool.emplace_back([&map]() {
			for (int i = 0; i < 100; ++i) {
				map.readNumber<int>(i, 0);
				map.readNumber<long long>(i, 4);
			}

			});
	};
	std::cout << "Thread completed" << std::endl;*/
	
	/*map.writeNumber<int>(0, 0, 2147483647);
	map.writeNumber<long long>(0, 4, 9223372036854775807);
	map.writeNumber<int>(1, 0, -2147483648);
	map.writeNumber<long long>(1, 4, -9223372036854775808);

	std::cout << map.readNumber<int>(0, 0) << std::endl;
	std::cout << map.readNumber<long long>(0, 4) << std::endl;
	std::cout << map.readNumber<int>(1, 0) << std::endl;
	std::cout << map.readNumber<long long>(1, 4) << std::endl;*/

	return 0;
}
