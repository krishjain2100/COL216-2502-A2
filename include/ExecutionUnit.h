#pragma once
#include <vector>
#include "Basics.h"
#include "ReservationStation.h"

inline constexpr long long INF = 2147483647LL;
inline constexpr long long NEGINF = -2147483648LL;

class ExecutionUnit {
public:
    ReservationStation rs;
    int latency;
    
    std::vector<PipelineMember> pipeline; 
    std::vector<Broadcast> ready_to_broadcast; 

    ExecutionUnit();
    ExecutionUnit(int _latency, int rs_size);

    void flush();
    void dispatch();
    void executeCycle();
};

