#include "../include/ExecutionUnit.h"

ExecutionUnit::ExecutionUnit() : rs(0), latency(0) {}
ExecutionUnit::ExecutionUnit(int _latency, int rs_size) : rs(rs_size), latency(_latency) {
    pipeline.resize(_latency);
}

void ExecutionUnit::flush() {
    for (auto& stage : pipeline) {
        stage.valid = false;
        stage.parent = nullptr;
    }
    ready_to_broadcast.clear();
    rs.clear();
}

void ExecutionUnit::dispatch() {
    if (pipeline[0].valid) return;
    RSEntry* entry_to_dispatch = nullptr;
    for(auto &entry : rs.entries) {
        if (entry.valid and !entry.executing and entry.Qj == -1 and entry.Qk == -1) {
            if(entry_to_dispatch == nullptr ) {
                entry_to_dispatch = &entry;
            }
            else if(entry_to_dispatch->issued_cycle > entry.issued_cycle){
                entry_to_dispatch = &entry;
            }
        }
    }
    if(entry_to_dispatch) {
        entry_to_dispatch->executing = true; 
        pipeline[0].parent = entry_to_dispatch;
        pipeline[0].valid = true;
    }
}

void ExecutionUnit::executeCycle() {
    if (pipeline.back().valid) {
        RSEntry* parent = pipeline.back().parent;
        int result = 0;
        bool exception = false;
        long long temp_res = 0;
        switch (parent->op) {
            case OpCode::ADD:
                temp_res = (long long)parent->Vj + (long long)parent->Vk;
                if (temp_res > INF or temp_res < NEGINF) exception = true;
                result = (int)temp_res;
                break;
            case OpCode::ADDI: 
                temp_res = (long long)parent->Vj + parent->A; 
                if (temp_res > INF or temp_res < NEGINF) exception = true;
                result = (int)temp_res;
                break;
            case OpCode::SUB:  
                temp_res = (long long)parent->Vj - (long long)parent->Vk;
                if (temp_res > INF or temp_res < NEGINF) exception = true;
                result = (int)temp_res;
                break;
            case OpCode::MUL:  
                temp_res = (long long)parent->Vj * (long long)parent->Vk;
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

            case OpCode::SLT: result = (parent->Vj < parent->Vk) ? 1 : 0; break;
            case OpCode::SLTI: result = (parent->Vj < parent->A) ? 1 : 0; break;
            case OpCode::AND: result = parent->Vj & parent->Vk; break;
            case OpCode::ANDI: result = parent->Vj & parent->A; break;
            case OpCode::OR: result = parent->Vj | parent->Vk; break;
            case OpCode::ORI: result = parent->Vj | parent->A; break;
            case OpCode::XOR: result = parent->Vj ^ parent->Vk; break;
            case OpCode::XORI: result = parent->Vj ^ parent->A; break;

            case OpCode::BEQ: result = (parent->Vj == parent->Vk) ? 1 : 0; break;
            case OpCode::BNE: result = (parent->Vj != parent->Vk) ? 1 : 0; break;
            case OpCode::BLT: result = (parent->Vj < parent->Vk) ? 1 : 0; break;
            case OpCode::BLE: result = (parent->Vj <= parent->Vk) ? 1 : 0; break;
            case OpCode::J: result = 1; break;
                
            default: break;
        }
        parent->valid = false;
        rs.sz--;
        ready_to_broadcast.push_back({parent->rob_tag, result, true, exception}); 
    }

    for (int i = latency - 1; i > 0; i--) {
        pipeline[i] = pipeline[i - 1];
    }
    
    pipeline[0].valid = false;
    pipeline[0].parent = nullptr;
}


