#pragma once
#include "files/Tabble.h"
#include <vector>
#include <unordered_map>
#include <shared_mutex>


//TO DO: Добавить подгрузку данных самой базы данных при запуске
class DataBase {
private:

	

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


	Result optimize(QueryVariant& query_var) {
		return std::visit([this](auto& query) {
			// Всё внутри visit
			size_t id = getTabbleId(query.tabble_name);
			Tabble& tabble = *tabbles[id];
			auto info = tabble.getInfo();
			query.optimizeImpl(info);
			return query.execute(tabble, info);
			}, query_var);
	}

	
};