// SQLid.cpp: определяет точку входа для приложения.
//

#include "SQLid.h"
#include <thread>
#include "files/InfoLoader.h"


int main()
{
	

	std::string name = "C:\\Users\\kuzne\\Desktop\\PetProjects\\SQLid\\out\\build\\x64-release\\test_info.db";
	InfoLoader loader(name);

	auto el = loader.loadNewInfo();

	std::cout << el->lineLength<<std::endl;
	std::cout << el->columnsNumber << std::endl;
	for (auto& s : el->columns) {
		std::cout << "NAME: "<<s.name << " OFFSET: " << s.offset << " SIZE: " << s.size << std::endl;
	}
	
	int a;
	std::cin >> a;

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
