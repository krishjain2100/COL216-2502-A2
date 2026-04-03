#include "../include/LoadStoreQueue.h"

LoadStoreQueue::LoadStoreQueue() : latency(0), rs_size(0) {}
LoadStoreQueue::LoadStoreQueue(int _latency, int _rs_size) : latency(_latency), rs_size(_rs_size) {}

int LoadStoreQueue::getSize() const {
    return lsq.size();
}

bool LoadStoreQueue::isFull() const {
    return getSize() >= rs_size;
}

bool LoadStoreQueue::insert(const RSEntry &entry) {
    if (isFull()) return false;
    lsq.push_back(entry);
    return true;
}

void LoadStoreQueue::listen(const CDB& bus) {
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

void LoadStoreQueue::flush() {
    inPipetIns.clear();
    ready_to_broadcast.clear();
    lsq.clear();
}

void LoadStoreQueue::pop() {
    if(lsq.empty()) return;
    lsq.pop_front();
}

void LoadStoreQueue::dispatch() {
    for (auto& entry : lsq) {
        if(entry.executing) continue;
        if(entry.Qj == -1 && entry.Qk == -1) {
            LSQInPipeline new_op;
            new_op.cycles_remaining = latency;
            new_op.parent = &entry;
            inPipetIns.push_back(new_op);
            entry.executing = true; 
        }
        break;
    }
}

void LoadStoreQueue::executeCycle(const std::vector<int>& Memory) {
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
            }
            ready_to_broadcast.push_back({ parent->rob_tag, result, true, exception });
            it = inPipetIns.erase(it);
        } 
        else {
            it++;
        }
    }
}