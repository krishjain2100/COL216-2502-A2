#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Basics.h"

const long long INF = 2147483647LL;
const long long NEGINF = -2147483648LL;

struct RSInPipeline {
    int cycles_remaining;
    RSEntry *parent;
};

class ExecutionUnit {
public:
    ReservationStation rs;
    int latency;

    std::vector<RSInPipeline> inPipetIns; 
    std::vector<Broadcast> ready_to_broadcast; 

    ExecutionUnit() {} 

    ExecutionUnit(int _latency, int rs_size)
        : rs(rs_size), latency(_latency) {}

    void flush() {
        inPipetIns.clear();
        ready_to_broadcast.clear();
        rs.clear();
    }

    void executeCycle() {
        for (auto it = inPipetIns.begin(); it != inPipetIns.end(); ) {
            it->cycles_remaining--;
            RSEntry* parent = it->parent;
            if (it->cycles_remaining <= 0) {
                int result = 0;
                bool exception = false;
                long long temp_res = 0; 
                switch (parent->op) {
                    case OpCode::ADD:
                    case OpCode::ADDI: 
                        temp_res = parent->Vj + parent->Vk;
                        if (temp_res > INF or temp_res < NEGINF) exception = true;
                        result = (int)temp_res;
                        break;
                    case OpCode::SUB:  
                        temp_res = parent->Vj - parent->Vk;
                        if (temp_res > INF or temp_res < NEGINF) exception = true;
                        result = (int)temp_res;
                        break;
                    case OpCode::MUL:  
                        temp_res = parent->Vj * parent->Vk;
                        if (temp_res > INF or temp_res < NEGINF) exception = true;
                        result = (int)temp_res;
                        break;
                    case OpCode::DIV: 
                        if (parent->Vk == 0) exception = true;
                        else result = parent->Vj / parent->Vk; 
                        break;
                    case OpCode::REM: 
                        if (parent->Vk == 0) exception = true;
                        else result = parent->Vj % parent->Vk; 
                        break;

                    case OpCode::SLT:
                    case OpCode::SLTI:
                        result = (parent->Vj < parent->Vk) ? 1 : 0;
                        break;
                    case OpCode::AND:
                    case OpCode::ANDI:
                        result = parent->Vj & parent->Vk;
                        break;
                    case OpCode::OR:
                    case OpCode::ORI:
                        result = parent->Vj | parent->Vk;
                        break;
                    case OpCode::XOR:
                    case OpCode::XORI:
                        result = parent->Vj ^ parent->Vk;
                        break;

                    // 1 (taken) or 0 (not taken). commparent will use this to verify the BranchPredictor's guess.
                    case OpCode::BEQ: result = (parent->Vj == parent->Vk) ? 1 : 0; break;
                    case OpCode::BNE: result = (parent->Vj != parent->Vk) ? 1 : 0; break;
                    case OpCode::BLT: result = (parent->Vj < parent->Vk) ? 1 : 0; break;
                    case OpCode::BLE: result = (parent->Vj <= parent->Vk) ? 1 : 0; break;
                        
                    default: break;
                }

                ready_to_broadcast.push_back({parent->rob_tag, result, true, exception}); 
                it = inPipetIns.erase(it);
            } 
            else {
                it++;
            }
        }

        for(auto& entry : rs.entries) {
            if (entry.busy and entry.Qj == -1 and entry.Qk == -1) {
                RSInPipeline new_op;
                new_op.cycles_remaining = latency;
                new_op.parent = &entry;
                inPipetIns.push_back(new_op);
                entry.busy = false; 
                break; 
            }
        }
    }
};

