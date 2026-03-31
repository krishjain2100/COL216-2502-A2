#pragma once
#include <string>
#include <vector>

enum class OpCode { ADD, SUB, ADDI, MUL, DIV, REM, LW, SW, BEQ, BNE, BLT, BLE, J, SLT, SLTI, AND, OR, XOR, ANDI, ORI, XORI };
enum class UnitType { ADDER, MULTIPLIER, DIVIDER, LOADSTORE, BRANCH, LOGIC };

std::vector <OpCode> branchOp = {OpCode::BEQ, OpCode::BNE, OpCode::BLT, OpCode::BNE};

struct Instruction {
    OpCode op;
    int dest = -1;
    int src1 = -1;
    int src2 = -1;
    int imm = 0;
    int pc = -1;
};

struct ProcessorConfig {
    int num_regs = 32;
    int rob_size = 64;
    int mem_size = 1024;

    int add_lat = 2;
    int mul_lat = 4;
    int div_lat = 5;
    int mem_lat = 4;
    // int br_lat = ??;
    // int logic_lat = ??;

    int adder_rs_size = 4;
    int mult_rs_size = 2;
    int div_rs_size = 2;
    int lsq_rs_size = 32;
    int br_rs_size = 2;
    // int logic_rs_size = ??;

};


struct Broadcast {
    int tag;
    int value;
    bool valid;
    bool exception;
};

struct CDB {
    Broadcast current;
    void broadcast(int tag, int value, bool exception) {
        current = {tag, value, true, exception};
    }
    void clear() {
        current.valid = false;
    }
};

struct ROBEntry {
   bool busy = false;
   Instruction ins;
   int dest; // reg number
   int value; 
   bool exception = false;
   bool mispredicted = false;
   int predicted_pc = 0;
};

struct RSEntry {
    UnitType unit;
    OpCode op;
    int Qj = -1;
    int Qk = -1;
    int Vj;
    int Vk;
    int A; // for lw-sw, first imm then actual address
    int rob_tag;
    bool busy = false; // validates the RSEntry and checks if dispacted for LSQ
};






