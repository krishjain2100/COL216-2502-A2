#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "Basics.h"
#include "ExecutionUnit.h"
#include "ReorderBuffer.h"
#include "LoadStoreQueue.h"
#include "BranchPredictor.h"
#include "CommonDataBus.h"
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
    CDB bus;
    BranchPredictor bp;

    Instruction* current_ins;
    
    Processor(ProcessorConfig& config);
    void loadProgram(const std::string& filename);

    bool isBranchInstruction(OpCode &op) const;
    UnitType getUnitForOpcode(const OpCode op) const;

    void stageFetch();
    bool stageDecode();
    void stageExecuteAndBroadcast();
    void stageCommit();
    void flush();
    bool step();
    void dumpArchitecturalState();
};
