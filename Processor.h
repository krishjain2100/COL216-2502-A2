#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "Basics.h"
#include "BranchPredictor.h"
#include "ExecutionUnit.h"
#include "LoadStoreQueue.h"
#include "Parser.h"

class Processor {
public:
    int pc;
    int next_pc;
    int pc_limit;
    int clock_cycle;

    std::vector<Instruction> inst_memory;
    std::vector<RSEntry> ready_for_execution;
    
    std::vector<int> ARF; // regFile
    std::vector<int> Memory; // Memory
    bool exception = false; // exception bit

    std::map <UnitType, ExecutionUnit> eus;
    std::vector<int> RAT;

    ReorderBuffer ROB;
    LoadStoreQueue LSQ;
    map <int, BranchPredictor> bp;

    Instruction* current_ins;

    Processor(ProcessorConfig& config) {
        pc = 0;
        next_pc = 0;
        clock_cycle = 0;

        ARF.resize(config.num_regs, 0); // regs are initiliased to zero
        RAT.resize(config.num_regs, -1);
        Memory.resize(config.mem_size, 0);

        // Instantiate Hardware Units
        eus[UnitType::ADDER] = ExecutionUnit(add_lat, adder_rs_size);
        eus[UnitType::MULTIPLIER] = ExecutionUnit(mul_lat, mult_rs_size);
        eus[UnitType::DIVIDER] = ExecutionUnit(div_lat, div_rs_size);
        eus[UnitType::BRANCH] = ExecutionUnit(br_lat, br_rs_size);
        eus[UnitType::LOGIC] = ExecutionUnit(logic_lat, logic_rs_size);
        LSQ = new LoadStoreQueue(LSQ_rs_size, mem_lat);
    }

    void loadProgram(const std::string& filename) {
        std::ifstream f(filename);
        Parser p(f, inst_memory, Memory);
        pc_limit = inst_memory.size();
    }

    bool isBranchInstruction(OpCode &op) const {
        for(auto &o : branchOp) {
            if(o == op) return true;
        }
        return false;
    }

    void stageFetch() {
        if(ROB.isFull()) current_ins = nullptr;
        pc = next_pc;
        current_ins = &inst_memory[pc];
        if(isBranchInstruction(current_ins->op)) {
            next_pc = bp[pc].predict(pc, current_ins->imm, current_ins->op);
        }
        else {
            next_pc = pc + 1;
        }
    };

    UnitType getUnitForOpcode(const OpCode op) const { 
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

    bool stageDecode() {
        if (ROB.isFull()) {
            return false; // stall
        }

        UnitType unit = getUnitForOpcode(current_ins.op); 

        if (unit == UnitType::LOADSTORE) {
            if (LSQ.isFull()) return false; 
        } else {
            if (eus[unit].rs.isFull()) return false; 
        }

        int tag = ROB.getNextTag();
        int Qj = (current_ins->src1 >= 0) ? RAT[current_ins->src1] : -1;
        int Qk = (current_ins->src2 >= 0) ? RAT[current_ins->src2] : -1;
        int Vj = (Qj == -1 and current_ins->src1 >= 0) ? ARF[current_ins->src1] : -1;
        int Vk = (Qk == -1 and current_ins->src2 >= 0) ? ARF[current_ins->src2] : -1;


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
            RAT[dest] = tag;
        }

        return true; 
    };

    void stageExecuteAndBroadcast() {
        for (auto& [type, eu] : eus) {
            eu.executeCycle();
        }
        LSQ.executeCycle(Memory);
        for (auto& bc : eu.ready_to_broadcast) {
            bus.broadcast(bc.tag, bc.value, bc.exception);
            for (auto& [type, eu] : eus) {
                eu.rs.listen(bus);
            }
            ROB.listen(bus);
            LSQ.listen(bus);
        }
        eu.ready_to_broadcast.clear();
    };

    void stageCommit() {
        if (ROB.isEmpty()) return;

        ROBEntry &head = ROB.buffer[ROB.left];

        if (head.busy) return; // stall

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
                correct_pc = head.ins.pc + head.ins.imm;
            } else {
                correct_pc = head.ins.pc + 1;
            }

            if (head.predicted_pc != correct_pc) {
                bp[pc].update(actually_taken, false, head.ins.op); 
                pc = head.ins.pc;
                next_pc = correct_pc;
                flush();         
                return;
            } else {
                bp[pc].update(actually_taken, true, head.ins.op);
            }
        }

        if (head.dest > 0) {
            ARF[head.dest] = head.value;
        }

        if (head.ins.op == OpCode::SW) {
            int address = LSQ.LSQ.front().A;
            Memory[address] = head.value; 
        }

        if (head.ins.op == OpCode::LW or head.ins.op == OpCode::SW) {
            LSQ.pop();
        }
        ROB.remove();
    }

    void flush() {
        for (auto& [type, eu] : eus) {
            eu.flush();
        }
        LSQ.flush();
        ROB.flush();
        for(auto &e : RAT) e = -1;
        bus.clear();
        current_ins = nullptr;
    };

    bool step() { 
        clock_cycle++;
        stageFetch();
        stageDecode();
        stageExecuteAndBroadcast();
        stageCommit();
        if(next_pc >= pc_limit) return false;
        return true;
    }

    void dumpArchitecturalState() {
        std::cout << "\n=== ARCHITECTURAL STATE (CYCLE " << clock_cycle << ") ===\n";
        for (int i = 0; i < ARF.size(); i++) {
            std::cout << "x" << i << ": " << std::setw(4) << ARF[i] << " | ";
            if ((i+1) % 8 == 0) std::cout << std::endl;
        }
        if (exception) {
            std::cout << "EXCEPTION raised by instruction " << pc + 1 << std::endl;
        }
        for(auto &[pc, p] : bp)
            std::cout << "Branch Predictor Stats: " << p.correct_predictions << "/" << p.total_branches << " correct.\n";
    }
};