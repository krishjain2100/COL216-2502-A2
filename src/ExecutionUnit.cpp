#include "../include/ExecutionUnit.h"

ExecutionUnit::ExecutionUnit() : rs(0), latency(0) {}
ExecutionUnit::ExecutionUnit(int _latency, int rs_size) : rs(rs_size), latency(_latency) {}

void ExecutionUnit::flush() {
    inPipetIns.clear();
    ready_to_broadcast.clear();
    rs.clear();
}

void ExecutionUnit::dispatch() {
    for(auto &entry : rs.entries) {
        if (entry.busy and !entry.executing and entry.Qj == -1 and entry.Qk == -1) {
            if(entry_to_dispatch == nullptr ) {
                entry_to_dispatch = &entry;
            }
            else if(entry_to_dispatch->issued_cycle > entry.issued_cycle){
                entry_to_dispatch = &entry;
            }
        }
    }
    if(entry_to_dispatch) {
        RSInPipeline new_op;
        new_op.cycles_remaining = latency;
        entry_to_dispatch->executing = true; 
        new_op.parent = entry_to_dispatch;
        inPipetIns.push_back(new_op);
        entry_to_dispatch = nullptr;
    }
}

void ExecutionUnit::executeCycle() {
    for (auto it = inPipetIns.begin(); it != inPipetIns.end(); ) {
        it->cycles_remaining--;
        RSEntry* parent = it->parent;
        if (it->cycles_remaining <= 0) {
            int result = 0;
            bool exception = false;
            long long temp_res = 0; 
            switch (parent->op) {
                case OpCode::ADD:
                    temp_res = parent->Vj + parent->Vk;
                    if (temp_res > INF or temp_res < NEGINF) exception = true;
                    result = (int)temp_res;
                    break;
                case OpCode::ADDI: 
                    temp_res = parent->Vj + parent->A; 
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
            parent->busy = false;
            rs.sz--;
            ready_to_broadcast.push_back({parent->rob_tag, result, true, exception}); 
            it = inPipetIns.erase(it);
        } 
        else {
            it++;
        }
    }
}

