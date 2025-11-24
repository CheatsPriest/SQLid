
#include "clients/Operator.h"
#include "SQLid.h"
#include <thread>
#include "files/InfoLoader.h"
#include "parser/Conditions.h"
#include "parser/RequestParser.h"
#include "data_base/DataBase.h"
#include "data_base/Executor.h"
#include <chrono>
#include "system/system.h"
#include "clients/SyncServer.h"


//cmake .. -DCMAKE_CXX_STANDARD=23 
//make -j4
//./SQLid 

int main(int argc, char* argv[])
{     
	std::string path;
	int port = 0;

	if (argc == 3) {
		try {
			port = std::stoi(argv[1]);
			path = argv[2];
		}
		catch (const std::exception& e) {
			std::cerr << "Invalid arguments. Usage: " << argv[0] << " [port] [path]" << std::endl;
			std::cerr << "Or run without arguments for interactive mode." << std::endl;
			return 1;
		}
	}
	
	else if (argc == 1) {
		std::cout << "SQLid Server Setup" << std::endl;
		std::cout << "==================" << std::endl;

		std::cout << "Enter port: ";
		std::cin >> port;

		std::cout << "Enter working path: ";
		std::cin >> path;
	}
	else {
		std::cerr << "Usage: " << argv[0] << " [port] [path]" << std::endl;
		std::cerr << "Examples:" << std::endl;
		std::cerr << "  " << argv[0] << " 8080 ./data" << std::endl;
		std::cerr << "  " << argv[0] << " (for interactive mode)" << std::endl;
		return 1;
	}

	
	if (port <= 0 or port > 65535) {
		std::cerr << "Error: Port must be between 1 and 65535" << std::endl;
		return 1;
	}

	boost::asio::io_context cont;
	SyncServer server(cont, port, path);
	
	for (int w = 1; w!=0;) {
		std::cout << "ENTER 0 TO TURN OFF" << std::endl;
		std::cin >> w;    
	}  
	server.shutdownAll();


	//Archeology museum
	//Operator oper;//ddkb    b         
	//ClientInfo local(1, nullptr);  
	//std::string buf;
	//local.baseId = 0;
	//oper.sys.deleteDatabase("base11");
	//oper.sys.restoreDatabase("base1backup", "base1");
	     
	//oper.createDataBase("TestCreated");
	//std::stringstream stream("TestCreated newtest result BOOL time INT64 name STRING[10] velocity FLOAT month TEXT");
	//oper.createTableInDataBase(stream);
	//while (true) {
	//	std::getline(std::cin, buf);    
	//	if (buf == "0" or buf=="EXIT")break;   
	//	auto request = oper.execute(local, buf);//for example "SELECT age name FROM test"
	//	request.print();
	//}
	

	//Executor exec;
	//System sys("C:/TestDataBase");

	//DataBase& base = sys.getDataBase("base1");

	////DataBase base("C:/TestDataBase/base1");
	////Executor exec;

	//////base.openTabble("C:/TestDataBase/test", "test");

	//RequestParser parser;

	//auto update1 = parser.parse("UPDATE test SET name = ""BUBU"" WHERE id == 4");
	//auto insert1 = parser.parse("INSERT INTO test 100 WUWU");
	//auto select1 = parser.parse("SELECT age name FROM test");
	//auto delete1 = parser.parse("DELETE FROM test WHERE age == 100");

	//auto insert2 = parser.parse("INSERT INTO nums 24252525");
	//base.optimizing(insert2);
	//

	//base.optimizing(update1);
	//base.optimizing(insert1);
	//base.optimizing(select1);
	//base.optimizing(delete1);

	//Result insertRes1, selectRes1, deleteRes1, updateRes1, insertRes2;

	//exec.execute(update1, updateRes1);
	//exec.execute(insert1, insertRes1);
	//exec.execute(delete1, deleteRes1);
	//exec.execute(select1, selectRes1);
	//exec.execute(insert2, insertRes2);


	//selectRes1.print();

	/*for (auto& names : selectRes1.types) {
		std::cout << names << " ";
	}
	std::cout << std::endl;
	for (auto& row : selectRes1.body) {
		if (row.size() == 0)break;
		std::cout << std::get<int64_t>(row[0])<<" "<<" "<< std::get<int32_t>(row[1])<<" "<< std::get<std::string>(row[2]) << std::endl;
		std::cout << std::endl;
	}*/

	//auto del1 = parser.parse("DELETE FROM test WHERE age == 100");
	//auto replyDel1 = base.optimize(del1);

	////auto result = parser.parse("SELECT age FROM test WHERE age == 10 AND ID == 1 AND age == 34 OR name == ""BABA"" LIMIT 100");
	//auto result = parser.parse("SELECT age name FROM test WHERE age < 100 OR name == ""NEWa""");
	//auto reply = base.optimize(result);

	//
	//
	//for (auto& row : reply.body) {
	//	if (row.size() == 0)break;
	//	std::cout << std::get<int64_t>(row[0])<<" "<<" "<< std::get<int32_t>(row[1])<<" "<< std::get<std::string>(row[2]) << std::endl;
	//	std::cout << std::endl;
	//}
	//

	//{
	//	auto insert = parser.parse("INSERT INTO test 100 ""NEWa""");
	//	base.optimize(insert);
	//}
	/*{
		auto insert = parser.parse("INSERT INTO test 31 ""SASA""");
		base.optimize(insert);
	}
	{
		auto insert = parser.parse("INSERT INTO test 32 ""VANA""");
		base.optimize(insert);
	}
	{
		auto insert = parser.parse("INSERT INTO test 33 ""SERG""");
		base.optimize(insert);
	}
	{
		auto insert = parser.parse("INSERT INTO test 34 ""VOVA""");
		base.optimize(insert);
	}
	{
		auto insert = parser.parse("INSERT INTO test 35 ""SHIL""");
		base.optimize(insert);
	}
	{
		auto insert = parser.parse("INSERT INTO test 10 ""BABA""");
		base.optimize(insert);
	}*/
	

	std::cout << 10 << std::endl;
	
	
	/*std::cout << result.tabble_name << std::endl;
	for (auto& el : result.columns_raw) {
		std::cout << "NAME " << el << std::endl;
	}
	for (auto& el : result.raw_conditions) {
		std::cout << "CONDOTION " << el << std::endl;
	}
	std::cout << result.limit << std::endl;*/

	

	/*
	

	std::cout << result.tabble_name << std::endl;
	for (auto& el : result.columns_raw) {
		std::cout <<"NAME "<< el << std::endl;
	} 
	for (auto& el : result.raw_conditions) {
		std::cout << "CONDOTION "<<el << std::endl;
	}
	std::cout << result.limit << std::endl;*/

	/*std::string files = "C:/TestDataBase/test";

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
