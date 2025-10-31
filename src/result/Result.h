#pragma once
#include <vector>
#include "supported_variants.h"

class Result {
private:


public:
	bool isSucces;
	std::vector<std::string> header;
	std::vector<std::string> types;
	std::string messeage;
	std::string error;
	std::vector<std::vector<variant_types>> body;
	Result() : isSucces(false) {};
};