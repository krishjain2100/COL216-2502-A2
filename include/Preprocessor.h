#pragma once
#include <string>
#include "StringUtils.h"

struct Entry {
	std::string text;
	bool is_mem_decl;
};

bool preprocess(const std::string& filename);