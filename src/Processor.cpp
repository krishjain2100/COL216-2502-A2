#include "../include/Processor.h"
#include <vector>

Processor::Processor(ProcessorConfig& config) {
    pc = 0;
    next_pc = 0;
    clock_cycle = 0;

    ARF.resize(config.num_regs, 0); // regs are initiliased to zero
    RAT.resize(config.num_regs, -1);
    Memory.resize(config.mem_size, 0);
    ROB.rob_size = config.rob_size;
    ROB.buffer.resize(config.rob_size);

    eus[UnitType::ADDER] = ExecutionUnit(config.add_lat, config.adder_rs_size);
    eus[UnitType::MULTIPLIER] = ExecutionUnit(config.mul_lat, config.mult_rs_size);
    eus[UnitType::DIVIDER] = ExecutionUnit(config.div_lat, config.div_rs_size);
    eus[UnitType::BRANCH] = ExecutionUnit(config.br_lat, config.br_rs_size);
    eus[UnitType::LOGIC] = ExecutionUnit(config.logic_lat, config.logic_rs_size);
    LSQ = LoadStoreQueue(config.mem_lat, config.lsq_rs_size);

    setupLogging();
}

Processor::~Processor() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

void Processor::loadProgram(const std::string& filename) {
    std::ifstream f(filename);
    Parser p(f, inst_memory, Memory);
    pc_limit = inst_memory.size();
}

bool Processor::isBranchInstruction(OpCode &op) const {
    for(auto &o : branchOp) {
        if(o == op) return true;
    }
    return false;
}

void Processor::stageFetch() {
    if (current_ins != nullptr) {
        return;
    }
    if (next_pc >= pc_limit) {
        current_ins = nullptr;
        return;
    }
    if(ROB.isFull()) {
        current_ins = nullptr;
        return;
    }
    pc = next_pc;
    current_ins = &inst_memory[pc];
    if(current_ins->op == OpCode::J) {
        next_pc = current_ins->imm;
    }
    else if(isBranchInstruction(current_ins->op)) {
        next_pc = bp.predict(pc, current_ins->imm);
    }
    else {
        next_pc = pc + 1;
    }
};

UnitType Processor::getUnitForOpcode(const OpCode op) const { 
    switch(op) {
        case OpCode::ADD: return UnitType::ADDER;
        case OpCode::SUB: return UnitType::ADDER;
        case OpCode::ADDI: return UnitType::ADDER; 
        case OpCode::MUL: return UnitType::MULTIPLIER;
        case OpCode::DIV: return UnitType::DIVIDER;
        case OpCode::REM: return UnitType::DIVIDER;
        case OpCode::LW: return UnitType::LOADSTORE;
        case OpCode::SW: return UnitType::LOADSTORE;
        case OpCode::BEQ: return UnitType::BRANCH;
        case OpCode::BNE: return UnitType::BRANCH;
        case OpCode::BLT: return UnitType::BRANCH;
        case OpCode::BLE: return UnitType::BRANCH;
        case OpCode::J: return UnitType::BRANCH;
        case OpCode::SLT: return UnitType::ADDER;
        case OpCode::SLTI: return UnitType::ADDER;
        case OpCode::AND: return UnitType::LOGIC;
        case OpCode::OR: return UnitType::LOGIC;
        case OpCode::XOR: return UnitType::LOGIC;
        case OpCode::ANDI: return UnitType::LOGIC;
        case OpCode::ORI: return UnitType::LOGIC;
        case OpCode::XORI: return UnitType::LOGIC;
        default: throw;
    }
}

void Processor::stageDecode() {
    if (current_ins == nullptr) return;
    if (ROB.isFull()) return;

    UnitType unit = getUnitForOpcode(current_ins->op); 

    if (unit == UnitType::LOADSTORE) {
        if (LSQ.isFull()) return; 
    } else {
        if (eus[unit].rs.isFull()) return; 
    }

    int tag = ROB.getNextTag();
    int Qj = (current_ins->src1 >= 0) ? RAT[current_ins->src1] : -1;
    int Qk = (current_ins->src2 >= 0) ? RAT[current_ins->src2] : -1;
    int Vj = (Qj == -1 and current_ins->src1 >= 0) ? ARF[current_ins->src1] : -1;
    int Vk = (Qk == -1 and current_ins->src2 >= 0) ? ARF[current_ins->src2] : -1;

    if (Qj != -1) {
        if (!ROB.buffer[Qj].busy) { 
            Vj = ROB.buffer[Qj].value;
            Qj = -1;
        }
    }

    if (Qk != -1) {
        if (!ROB.buffer[Qk].busy) {
            Vk = ROB.buffer[Qk].value;
            Qk = -1;
        }
    }

    RSEntry rs_entry;
    rs_entry.unit = unit;
    rs_entry.op = current_ins->op;
    rs_entry.Qj = Qj;
    rs_entry.Qk = Qk;
    rs_entry.Vj = Vj;
    rs_entry.Vk = Vk;
    rs_entry.A = current_ins->imm;
    rs_entry.rob_tag = tag;
    rs_entry.busy = !(unit == UnitType::LOADSTORE);

    ROBEntry rb_entry;
    rb_entry.busy = true;
    rb_entry.ins = *current_ins;
    rb_entry.dest = current_ins->dest;
    rb_entry.value = 0;      // Placeholder, calculated later   
    rb_entry.exception = false;
    rb_entry.mispredicted = false;
    rb_entry.predicted_pc = next_pc;

    if (unit == UnitType::LOADSTORE) {
        LSQ.insert(rs_entry);
    } else {
        eus[unit].rs.insert(rs_entry);
    }

    ROB.insert(rb_entry);

    if(current_ins->dest > 0) {
        RAT[current_ins->dest] = tag;
    }

    current_ins = nullptr; 
};

void Processor::stageExecuteAndBroadcast() {
    for(auto & [type, eu] : eus) {
        eu.dispatch();
    }
    LSQ.dispatch();

    for (auto& [type, eu] : eus) {
        eu.executeCycle();
    }
    LSQ.executeCycle(Memory);

    std::vector<Broadcast> to_broadcast;

    for(auto &[type, eu] : eus) {
        for (auto& bc : eu.ready_to_broadcast) {
            to_broadcast.push_back(bc);
        }
        eu.ready_to_broadcast.clear();
    }

    for (auto& bc : LSQ.ready_to_broadcast) {
        to_broadcast.push_back(bc);
    }
    LSQ.ready_to_broadcast.clear();

    for (auto& bc : to_broadcast) {
        bus.broadcast(bc.tag, bc.value, bc.exception);
        for (auto& [t, eu] : eus) {
            eu.rs.listen(bus);
        }
        LSQ.listen(bus);
        ROB.listen(bus);
    }

    bus.clear();
};

void Processor::stageCommit() {
    if(ROB.isEmpty()) return;
    ROBEntry &head = ROB.buffer[ROB.left];

    if (head.busy) return; 

    if (head.exception) {
        pc = head.ins.pc; 
        exception = true;        
        flush();            
        return;                
    }

    bool is_branch = isBranchInstruction(head.ins.op);
    if (is_branch) {
        bool actually_taken = (head.value == 1);
        int correct_pc;
        if (actually_taken) {
            correct_pc = head.ins.imm;
        } else {
            correct_pc = head.ins.pc + 1;
        }

        if (head.ins.op == OpCode::J) {
             if (head.predicted_pc != correct_pc) {
                 next_pc = correct_pc;
                 flush();
                 return;
             }
        }

        else {
            if (head.predicted_pc != correct_pc) {
                bp.update(head.ins.pc, actually_taken, false, head.ins.op); 
                pc = head.ins.pc;
                next_pc = correct_pc;
                flush();         
                return;
            } else {
                bp.update(head.ins.pc, actually_taken, true, head.ins.op);
            }

        }
        
    }

    if (head.dest > 0) {
        ARF[head.dest] = head.value;
        if (RAT[head.dest] == ROB.left) {
            RAT[head.dest] = -1; 
        }
    }

    if (head.ins.op == OpCode::SW) {
        int address = LSQ.lsq.front().A;
        Memory[address] = head.value; 
    }

    if (head.ins.op == OpCode::LW or head.ins.op == OpCode::SW) {
        LSQ.pop();
    }
    ROB.remove();
}

void Processor::flush() {
    for (auto& [type, eu] : eus) {
        eu.flush();
    }
    LSQ.flush();
    ROB.flush();
    for(auto &e : RAT) e = -1;
    bus.clear();
    current_ins = nullptr;
};

bool Processor::step() { 
    if (exception) return false; 
    clock_cycle++;
    stageCommit();
    stageExecuteAndBroadcast();
    stageDecode();
    stageFetch();

    logCycleState();

    if(next_pc >= pc_limit && ROB.isEmpty()) {
        return false;
    }
    return true;
}

void Processor::dumpArchitecturalState() {
    std::cout << "\n=== ARCHITECTURAL STATE (CYCLE " << clock_cycle << ") ===\n";
    for (int i = 0; i < ARF.size(); i++) {
        std::cout << "x" << i << ": " << std::setw(4) << ARF[i] << " | ";
        if ((i+1) % 8 == 0) std::cout << std::endl;
    }
    if (exception) {
        std::cout << "EXCEPTION raised by instruction " << pc + 1 << std::endl;
    }
    std::cout << "Branch Predictor Stats: " << bp.correct_predictions << "/" << bp.total_branches << " correct.\n";
}


void Processor::setupLogging() {
    namespace fs = std::filesystem;
    
    if (!fs::exists("logs")) {
        fs::create_directory("logs");
    }

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "logs/run_" << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S") << ".log";
    std::string filename = ss.str();

    log_file.open(filename);
    if (log_file.is_open()) {
        log_file << "Simulation started at: " << filename << "\n";
    }
}

void Processor::logCycleState() {
    if (!log_file.is_open()) return;

    log_file << "\n" << std::string(50, '=') << "\n";
    log_file << "CYCLE: " << clock_cycle << " | PC: " << pc << " | Next PC: " << next_pc << "\n";
    log_file << std::string(50, '-') << "\n";

    // 2. Log ROB Status to file
    log_file << "REORDER BUFFER (Head: " << ROB.left << ", Tail: " << ROB.right << "):\n";
    if (ROB.isEmpty()) {
        log_file << "  [Empty]\n";
    } else {
        int i = ROB.left;
        while (true) {
            ROBEntry &e = ROB.buffer[i];
            log_file << "  Tag " << i << " | Busy: " << (e.busy ? "Yes" : "No ") 
                      << " | Inst: " << opMapRev.at(e.ins.op) << " | Dest: x" << e.dest 
                      << " | Value: " << e.value << "\n";
            
            i = (i + 1) % ROB.rob_size;
            if (i == ROB.right) break;
        }
    }

    // 3. Log Reservation Stations to file
    log_file << "\nRESERVATION STATIONS:\n";
    for (auto& [type, eu] : eus) {
        log_file << "  Unit " << static_cast<int>(type) << " (Size: " << eu.rs.sz << "):\n";
        for (auto& entry : eu.rs.entries) {
            if (entry.busy) {
                log_file << "    [Tag " << entry.rob_tag << "] Op: " << opMapRev.at(entry.op)
                          << " | Qj: " << entry.Qj << " | Qk: " << entry.Qk 
                          << " | Vj: " << entry.Vj << " | Vk: " << entry.Vk << "\n";
            }
        }
    }

    // 4. Log LSQ to file
    log_file << "\nLOAD/STORE QUEUE:\n";
    if (LSQ.lsq.empty()) {
        log_file << "  [Empty]\n";
    } else {
        for (auto& entry : LSQ.lsq) {
            log_file << "    [Tag " << entry.rob_tag << "] Op: " << opMapRev.at(entry.op)
                      << " | Addr: " << entry.A << " | Busy: " << (entry.busy ? "Yes" : "No") << "\n";
        }
    }

    // 5. Log RAT to file
    log_file << "\nREGISTER ALIAS TABLE (RAT):\n  ";
    bool rat_empty = true;
    for (int i = 0; i < (int)RAT.size(); i++) {
        if (RAT[i] != -1) {
            log_file << "x" << i << "->Tag " << RAT[i] << " | ";
            rat_empty = false;
        }
    }
    if (rat_empty) log_file << "[All Clear]";
    log_file << "\n" << std::string(50, '=') << "\n";
    
    // Optional: Force the data to be written to the disk immediately
    log_file.flush(); 
}