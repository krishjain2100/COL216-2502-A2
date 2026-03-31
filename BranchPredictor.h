#pragma once
#include "Basics.h"
#include <iostream>
#include <map>
#include <vector>

class BranchPredictor {
public:
    int total_branches = 0;
    int correct_predictions = 0;
    int state = 0;

    int predict(int current_pc, int imm) { // returns the new pc
        int new_pc;
        if(state & 2) new_pc = current_pc + 1;
        else new_pc = current_pc + imm;
        return new_pc;
    }

    void update(bool taken, bool was_correct, OpCode op) {
        total_branches++;
        if (was_correct) {
            correct_predictions++;
        }
        if(state >= 1 and taken) {
            state--;
        }
        if(state <= 2 and !taken) {
            state++;
        }
    }
};