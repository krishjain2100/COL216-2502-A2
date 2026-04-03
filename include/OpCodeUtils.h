#pragma once
#include <vector>
#include <map>
#include "Basics.h"

inline const std::vector<OpCode> branchOp = {OpCode::BEQ, OpCode::BNE, OpCode::BLT, OpCode::BLE};
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

inline UnitType getUnitForOpcode(const OpCode &op) { 
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

inline bool isBranchInstruction(const OpCode &op) {
    for(auto &o : branchOp) {
        if(o == op) return true;
    }
    return false;
}