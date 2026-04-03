#pragma once
#include <vector>
#include "Basics.h"
#include "ReservationStation.h"

inline constexpr long long INF = 2147483647LL;
inline constexpr long long NEGINF = -2147483648LL;

struct RSInPipeline {
    int cycles_remaining;
    RSEntry* parent;
};

class ExecutionUnit {
public:
    ReservationStation rs;
    RSEntry* entry_to_dispatch = nullptr;
    int latency;

    std::vector<RSInPipeline> inPipetIns; 
    std::vector<Broadcast> ready_to_broadcast; 

    ExecutionUnit();
    ExecutionUnit(int _latency, int rs_size);

    void flush();
    void dispatch();
    void executeCycle();
};

