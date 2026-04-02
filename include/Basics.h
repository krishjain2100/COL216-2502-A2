#pragma once
#include <string>
#include <vector>
#include <map>


enum class OpCode { ADD, SUB, ADDI, MUL, DIV, REM, LW, SW, BEQ, BNE, BLT, BLE, J, SLT, SLTI, AND, OR, XOR, ANDI, ORI, XORI };
enum class UnitType { ADDER, MULTIPLIER, DIVIDER, LOADSTORE, BRANCH, LOGIC };

inline const std::vector<OpCode> branchOp = {OpCode::BEQ, OpCode::BNE, OpCode::BLT, OpCode::BLE, OpCode::J};
inline const std::map<std::string, OpCode> opMap = {
    {"add",  OpCode::ADD},  {"sub",  OpCode::SUB},  {"slt",  OpCode::SLT},
    {"addi", OpCode::ADDI},  {"slti", OpCode::SLTI},
    {"mul",  OpCode::MUL}, {"div",  OpCode::DIV}, {"rem",  OpCode::REM},
    {"and",  OpCode::AND},  {"or",   OpCode::OR},  {"xor",  OpCode::XOR},
    {"andi", OpCode::ANDI},  {"ori",  OpCode::ORI},   {"xori", OpCode::XORI},
    {"lw",   OpCode::LW},    {"sw",   OpCode::SW},  {"j",    OpCode::J},
    {"beq",  OpCode::BEQ},  {"bne",  OpCode::BNE}, {"blt",  OpCode::BLT},  {"ble",  OpCode::BLE},
};

inline const std::map<OpCode, std::string> opMapRev = {
    {OpCode::ADD, "add"}, {OpCode::SUB, "sub"}, {OpCode::MUL, "mul"},
    {OpCode::DIV, "div"}, {OpCode::LW, "lw"}, {OpCode::SW, "sw"},
    {OpCode::BEQ, "beq"}, {OpCode::BNE, "bne"}, {OpCode::ADDI, "addi"},
    {OpCode::J, "j"}, {OpCode::BLT, "blt"}, {OpCode::BLE, "ble"}
};

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

    int logic_lat = 1;
    int add_lat = 2;
    int mul_lat = 4;
    int div_lat = 5;
    int mem_lat = 4;
    int br_lat = 2; // not mentioned

    int logic_rs_size = 4;
    int adder_rs_size = 4;
    int mult_rs_size = 2;
    int div_rs_size = 2;
    int lsq_rs_size = 32;
    int br_rs_size = 2;
};

struct Broadcast {
    int tag;
    int value;
    bool valid;
    bool exception;
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
    bool busy = false; // validates the RSEntry and checks if dispatched from LSQ
    
};






