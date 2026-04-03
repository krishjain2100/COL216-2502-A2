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
        if (entry.busy and entry.Qj == -1 and entry.Qk == -1) {
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
        entry_to_dispatch->busy = false; 
        new_op.data = *entry_to_dispatch;
        inPipetIns.push_back(new_op);
        rs.sz--;
        entry_to_dispatch = nullptr;
    }
}

void ExecutionUnit::executeCycle() {
    for (auto it = inPipetIns.begin(); it != inPipetIns.end(); ) {
        it->cycles_remaining--;
        RSEntry data = it->data;
        if (it->cycles_remaining <= 0) {
            int result = 0;
            bool exception = false;
            long long temp_res = 0; 
            switch (data.op) {
                case OpCode::ADD:
                    temp_res = data.Vj + data.Vk;
                    if (temp_res > INF or temp_res < NEGINF) exception = true;
                    result = (int)temp_res;
                    break;
                case OpCode::ADDI: 
                    temp_res = data.Vj + data.A; 
                    if (temp_res > INF or temp_res < NEGINF) exception = true;
                    result = (int)temp_res;
                    break;
                case OpCode::SUB:  
                    temp_res = data.Vj - data.Vk;
                    if (temp_res > INF or temp_res < NEGINF) exception = true;
                    result = (int)temp_res;
                    break;
                case OpCode::MUL:  
                    temp_res = data.Vj * data.Vk;
                    if (temp_res > INF or temp_res < NEGINF) exception = true;
                    result = (int)temp_res;
                    break;
                case OpCode::DIV: 
                    if (data.Vk == 0) exception = true;
                    else result = data.Vj / data.Vk; 
                    break;
                case OpCode::REM: 
                    if (data.Vk == 0) exception = true;
                    else result = data.Vj % data.Vk; 
                    break;

                case OpCode::SLT: result = (data.Vj < data.Vk) ? 1 : 0; break;
                case OpCode::SLTI: result = (data.Vj < data.A) ? 1 : 0; break;
                case OpCode::AND: result = data.Vj & data.Vk; break;
                case OpCode::ANDI: result = data.Vj & data.A; break;
                case OpCode::OR: result = data.Vj | data.Vk; break;
                case OpCode::ORI: result = data.Vj | data.A; break;
                case OpCode::XOR: result = data.Vj ^ data.Vk; break;
                case OpCode::XORI: result = data.Vj ^ data.A; break;

                case OpCode::BEQ: result = (data.Vj == data.Vk) ? 1 : 0; break;
                case OpCode::BNE: result = (data.Vj != data.Vk) ? 1 : 0; break;
                case OpCode::BLT: result = (data.Vj < data.Vk) ? 1 : 0; break;
                case OpCode::BLE: result = (data.Vj <= data.Vk) ? 1 : 0; break;
                case OpCode::J: result = 1; break;
                    
                default: break;
            }

            ready_to_broadcast.push_back({data.rob_tag, result, true, exception}); 
            it = inPipetIns.erase(it);
        } 
        else {
            it++;
        }
    }
}

