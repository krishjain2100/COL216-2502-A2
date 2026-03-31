#pragma once
#include <vector>
#include <deque>
#include "Basics.h"
#include "CommonDataBus.h"

struct LSQInPipeline {
    int cycles_remaining = 0;
    RSEntry* parent = nullptr;
};
  
class LoadStoreQueue {
public:
    int latency;
    int rs_size;

    std::deque<RSEntry> lsq;
    std::vector<LSQInPipeline> inPipetIns;
    std::vector<Broadcast> ready_to_broadcast;

    LoadStoreQueue();
    LoadStoreQueue(int _latency, int _rs_size);

    int getSize() const;
    bool isFull() const;
    bool insert(const RSEntry &entry);
    void pop();

    void flush();
    void dispatch();
    void executeCycle(const std::vector<int>& Memory);
    void listen(const CDB& bus);
};