#pragma once
#include "Basics.h"
#include <map>

class BranchPredictor {
public:
	int total_branches = 0;
	int correct_predictions = 0;
	std::map <int, int> states;

	int predict(const int &current_pc, const int &imm);
	void update(const int &current_pc, const bool &taken, const bool &was_correct);
};