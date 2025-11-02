#pragma once
#include "files/Tabble.h"
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include "files/Optimizer.h"

//TO DO: ƒобавить подгрузку данных самой базы данных при запуске
class DataBase {
private:

	Optimizer optim;

	std::vector<std::unique_ptr<Tabble>> tabbles;

	
	std::unordered_map<std::string, size_t> tabbles_name;
	mutable std::shared_mutex map_mtx;

	size_t getTabbleId(const std::string& name) const {
		std::shared_lock<std::shared_mutex> lock(map_mtx);
		return tabbles_name.at(name);
		
	}

	void insertTabble(std::unique_ptr<Tabble> tabble, const std::string& name) {
		std::unique_lock<std::shared_mutex> lock(map_mtx);
		tabbles_name.insert({ name, tabbles.size() });
		tabbles.push_back(std::move(tabble));
	}

public:

	DataBase() {

		tabbles.reserve(16);

	};

	bool openTabble(std::string filename, std::string localName) {

		auto cur = std::make_unique<Tabble>(filename);
		
		insertTabble(std::move(cur), localName);

		return true;
	}


	void optimizing(QueryVariant& query_var) {

		std::string tabble_name = std::visit([](auto&& query) -> std::string {
			return query.tabble_name;  // работает благодар€ CRTP
			}, query_var);

		size_t id = getTabbleId(tabble_name);

		Tabble& tabble = *tabbles[id];
		auto info = tabble.getInfo();

		optim.optimize(query_var, tabble.getInfo(), tabble);

	}

	

	
};