#pragma once
#include <vector>
#include <deque>
#include <string>
#include "Basics.h"

struct LSQInPipeline {
    int cycles_remaining = 0;
    RSEntry* parent_entry = nullptr;
};
  
class LoadStoreQueue {
public:
    int latency;
    int rs_size;

    std::deque<RSEntry> lsq;
    std::vector<LSQInPipeline> inPipetIns;
    std::vector<Broadcast> ready_to_broadcast;

    LoadStoreQueue(int _latency, int _rs_size)
        : latency(_latency), rs_size(_rs_size) {}

    int getSize() const {
        return lsq.size();
    }


    bool isFull() const {
        return getSize() >= rs_size;
    }

    bool insert(const RSEntry &entry) {
        if (isFull()) return false;
        lsq.push_back(entry);
        return true;
    }

    void listen(const CDB& bus) {
        if (!bus.current.valid) return;
        for (auto& entry : lsq) {
            if (entry.Qj == bus.current.tag) {
                entry.Vj = bus.current.value;
                entry.Qj = -1;
            }
            if (entry.Qk == bus.current.tag) {
                entry.Vk = bus.current.value;
                entry.Qk = -1;
            }
        }
    }

    void flush() {
        inPipetIns.clear();
        ready_to_broadcast.clear();
        lsq.clear();
    }

    void pop() {
        if(lsq.empty()) return;
        lsq.pop_front();
    }

    void executeCycle(const std::vector<int>& Memory) {
        for (auto it = inPipetIns.begin(); it != inPipetIns.end(); ) {
            it->cycles_remaining--;
            RSEntry* parent = it->parent;
            if (it->cycles_remaining <= 0) {
                bool exception = false;
                int result = 0;
                parent->A = parent->Vj + parent->A; 
                if (parent->A < 0 or parent->A >= Memory.size()) {
                    exception = true;
                } 
                else {
                    if (parent->op == OpCode::LW) {
                        result = Memory[parent->A];
                    }
                    else {
                        result = parent->Vk;
                    }
                }
                ready_to_broadcast.push_back({parent->rob_tag, result, true, exception}); 
                it = inPipetIns.erase(it);
            } 
            else {
                it++;
            }
        }

        if (!lsq.empty()) {
            for (auto& entry : lsq) {
                if (!entry.busy) {
                    if (entry.Qj == -1 && entry.Qk == -1) {
                        LSQInPipeline new_op;
                        new_op.cycles_remaining = latency;
                        new_op.parent_entry = &entry;
                        inPipetIns.push_back(new_op);
                        entry.busy = true; 
                    }
                    break; 
                }
            }
        }
    }
};