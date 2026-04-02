#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
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
    int pc = 0;
    int next_pc;
    int pc_limit;
    int clock_cycle = 0;
    bool exception = false; 

    Instruction* current_ins = nullptr;

    std::vector<Instruction> inst_memory;
    std::vector<RSEntry> ready_for_execution;
    
    std::vector<int> ARF; 
    std::vector<int> Memory; 
    std::vector<int> RAT;
    std::map <UnitType, ExecutionUnit> EUS;

    ReorderBuffer ROB;
    LoadStoreQueue LSQ;
    CDB BUS;
    BranchPredictor BP;

    std::ofstream log_file;

    
    Processor(ProcessorConfig& config);
    ~Processor();
    void loadProgram(const std::string &filename);

    bool isBranchInstruction(const OpCode &op) const;
    UnitType getUnitForOpcode(const OpCode &op) const;

    void stageFetch();
    void stageDecode();
    void stageExecuteAndBroadcast();
    void stageCommit();
    void flush();
    bool step();
    void dumpArchitecturalState() const;
    void setupLogging();
    void logCycleState();
};
