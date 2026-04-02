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
    int pc;
    int next_pc;
    int pc_limit;
    int clock_cycle;

    std::vector<Instruction> inst_memory;
    std::vector<RSEntry> ready_for_execution;
    
    std::vector<int> ARF; 
    std::vector<int> Memory; 
    bool exception = false; 

    std::map <UnitType, ExecutionUnit> eus;
    std::vector<int> RAT;

    ReorderBuffer ROB;
    LoadStoreQueue LSQ;
    CDB bus;
    BranchPredictor bp;

    Instruction* current_ins = nullptr;


    std::ofstream log_file;

    
    Processor(ProcessorConfig& config);
    ~Processor();
    void loadProgram(const std::string& filename);

    bool isBranchInstruction(OpCode &op) const;
    UnitType getUnitForOpcode(const OpCode op) const;

    void stageFetch();
    void stageDecode();
    void stageExecuteAndBroadcast();
    void stageCommit();
    void flush();
    bool step();
    void dumpArchitecturalState();
    void setupLogging();
    void logCycleState();
};
