#pragma once
#include "Basics.h"
#include <map>

class BranchPredictor {
public:
    int total_branches = 0;
    int correct_predictions = 0;
    std::map <int, int> states;

    int predict(int current_pc, int imm); // returns the new pc
    void update(int current_pc, bool taken, bool was_correct, OpCode op);
};