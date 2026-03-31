#include <fstream>
#include <string>
#include <vector>
#include "Basics.h"

class Parser {
public:
    Parser(std::ifstream &file, std::vector<Instruction> &inst_memory, std::vector<int> &Memory) ;
    // "A" or "3" as a memory offset
    int resolveMemLabel(const std::string &s, const std::map<std::string, int> &mem_labels);
    // "A(x2)" or "4(x2)"
    void parseMemAccess(Instruction &inst, const std::string &operand, const std::map<std::string, int> &mem_labels);
    void parseOperands(Instruction &inst, const std::string &rest, const std::map<std::string, int> &mem_labels);
};