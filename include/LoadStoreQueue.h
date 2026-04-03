#pragma once
#include <vector>
#include "Basics.h"
#include "CommonDataBus.h"

class LoadStoreQueue {
public:
    int latency;
    int rs_size;
    int head = 0;  
    int tail = 0; 
    int sz = 0;

    std::vector<RSEntry> entries;
    std::vector<PipelineMember> pipeline;
    std::vector<Broadcast> ready_to_broadcast;

    LoadStoreQueue();
    LoadStoreQueue(int _latency, int _rs_size);

    int getSize() const;
    bool isFull() const;
    bool push(const RSEntry &entry);
    void pop();

    void flush();
    void dispatch();
    void executeCycle(const std::vector<int>& Memory);
    void listen(const CDB& bus);
};