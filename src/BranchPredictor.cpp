#include "../include/BranchPredictor.h"


int BranchPredictor::predict(int current_pc, int imm) { 
        int state = states[current_pc];
        int new_pc;
        if(state & 2) new_pc = current_pc + 1;
        else new_pc = current_pc + imm;
        return new_pc;
    }

void BranchPredictor::update(int current_pc, bool taken, bool was_correct, OpCode op) {
    total_branches++;
    if (was_correct) {
        correct_predictions++;
    }
    int state = states[current_pc];
    if(state >= 1 and taken) {
        state--;
    }
    if(state <= 2 and !taken) {
        state++;
    }
}