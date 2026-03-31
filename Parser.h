#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "Basics.h"
#include "StringUtils.h"
using namespace std;

static const map<string, OpCode> opMap = {
    {"add",  OpCode::ADD},  
    {"sub",  OpCode::SUB},  
    {"addi", OpCode::ADDI},
    {"slt",  OpCode::SLT}, 
    {"slti", OpCode::SLTI},
    {"mul",  OpCode::MUL},
    {"div",  OpCode::DIV},  
    {"rem",  OpCode::REM},
    {"and",  OpCode::AND},  
    {"or",   OpCode::OR},   
    {"xor",  OpCode::XOR},
    {"andi", OpCode::ANDI}, 
    {"ori",  OpCode::ORI},  
    {"xori", OpCode::XORI},
    {"lw",   OpCode::LW},   
    {"sw",   OpCode::SW},
    {"beq",  OpCode::BEQ},  
    {"bne",  OpCode::BNE},
    {"blt",  OpCode::BLT},  
    {"ble",  OpCode::BLE},
    {"j",    OpCode::J}
};

class Parser {
public:
    Parser(ifstream &file, vector<Instruction> &inst_memory, vector<int> &Memory) {
        vector<string> lines;
        for (string l; getline(file, l);)
            lines.push_back(stripComment(l));

        map<string, int> mem_labels; // e.g. "A" → 0,  "B" → 3
        int ptr = 0;

        for (const auto &line : lines) {
            if (line.empty() || line[0] != '.') continue;

            size_t colon = line.find(':');
            string label = trim(line.substr(1, colon - 1));
            mem_labels[label] = ptr;

            istringstream iss(line.substr(colon + 1));
            int val;
            while (iss >> val) {
                Memory[ptr] = val;
                ptr++;
            }
        }

        int pc = 0;
        for (const auto &line : lines) {
            if (line.empty() || line[0] == '.') continue; 
            istringstream iss(line);
            string opStr;
            iss >> opStr;
            Instruction inst;
            inst.op = opMap[opStr];
            inst.pc = pc++;
            string rest;
            getline(iss, rest);
            rest = trim(rest);
            parseOperands(inst, rest, mem_labels);
            inst_memory.push_back(inst);
        }
    }

    // "A" or "3" as a memory offset
    int resolveMemLabel(const string &s, const map<string, int> &mem_labels) {
        auto it = mem_labels.find(s);
        return it != mem_labels.end() ? it->second : stoi(s);
    }

    // "A(x2)" or "4(x2)"
    void parseMemAccess(Instruction &inst, const string &operand, const map<string, int> &mem_labels) {
        size_t lp = operand.find('(');
        size_t rp = operand.find(')');
        string offset_str = trim(operand.substr(0, lp));
        string reg_str = trim(operand.substr(lp + 1, rp - lp - 1));
        inst.imm  = resolveMemLabel(offset_str, mem_labels);
        inst.src1 = parseReg(reg_str);
    }

    void parseOperands(Instruction &inst, const string &rest, const map<string, int> &mem_labels) {
        switch (inst.op) {
            //  op  dest, src1, src2 ---
            case OpCode::ADD: 
            case OpCode::SUB:
            case OpCode::MUL: 
            case OpCode::DIV: 
            case OpCode::REM:
            case OpCode::SLT:
            case OpCode::AND: 
            case OpCode::OR:  
            case OpCode::XOR: {
                auto t = splitComma(rest);  // ["x1","x2","x3"]
                inst.dest = parseReg(t[0]);
                inst.src1 = parseReg(t[1]);
                inst.src2 = parseReg(t[2]);
                break;
            }

            //  op  dest, src1, imm 
            case OpCode::ADDI: 
            case OpCode::SLTI:
            case OpCode::ANDI: 
            case OpCode::ORI: 
            case OpCode::XORI: {
                auto t = splitComma(rest); // ["x1","x2","5"]
                inst.dest = parseReg(t[0]);
                inst.src1 = parseReg(t[1]);
                inst.imm  = stoi(t[2]);
                break;
            }

            // lw  dest, offset(base)
            case OpCode::LW: {
                auto t = splitComma(rest); // ["x4","A(x1)"]
                inst.dest = parseReg(t[0]);
                parseMemAccess(inst, t[1], mem_labels);
                break;
            }

            // sw  src2, offset(base) 
            case OpCode::SW: {
                auto t = splitComma(rest);  // ["x5","A(x1)"]
                inst.src2 = parseReg(t[0]);
                parseMemAccess(inst, t[1], mem_labels);
                break;
            }

            // branch: op  src1, src2, target_pc 
            case OpCode::BEQ: 
            case OpCode::BNE:
            case OpCode::BLT: 
            case OpCode::BLE: {
                auto t = splitComma(rest); // ["x1","x2","7"]
                inst.src1 = parseReg(t[0]);
                inst.src2 = parseReg(t[1]);
                inst.imm  = stoi(t[2]);
                break;
            }
            case OpCode::J: {
                inst.imm = stoi(trim(rest));
                break;
            }
            default: throw ;
        }
    }
};