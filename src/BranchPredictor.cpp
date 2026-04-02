#include "../include/BranchPredictor.h"

int BranchPredictor::predict(const int &current_pc, const int &imm) { 
        int &state = states[current_pc];
        int new_pc;
        if(state & 2) new_pc = current_pc + 1; // not taken
        else new_pc = imm; // taken
        return new_pc;
    }

void BranchPredictor::update(const int &current_pc, const bool &taken, const bool &was_correct, const OpCode &op) {
    total_branches++;
    if (was_correct) {
        correct_predictions++;
    }
    int &state = states[current_pc];
    if(state >= 1 and taken) {
        state--;
    }
    if(state <= 2 and !taken) {
        state++;
    }
}