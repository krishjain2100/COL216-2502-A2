#include "../include/LoadStoreQueue.h"

LoadStoreQueue::LoadStoreQueue() : latency(0), rs_size(0) {}
LoadStoreQueue::LoadStoreQueue(int _latency, int _rs_size) : latency(_latency), rs_size(_rs_size) {
    pipeline.resize(_latency);
}

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
    for (auto& stage : pipeline) {
        stage.valid = false;
        stage.parent = nullptr;
    }
    ready_to_broadcast.clear();
    lsq.clear();
}

void LoadStoreQueue::pop() {
    if(lsq.empty()) return;
    lsq.pop_front();
}

void LoadStoreQueue::dispatch() {
    if (pipeline[0].valid) return;
    for (auto& entry : lsq) {
        if(entry.executing) continue;
        if (entry.Qj == -1 && entry.Qk == -1) {
            entry.executing = true; 
            pipeline[0].parent = &entry;
            pipeline[0].valid = true;
        }
        break;
    }
}

void LoadStoreQueue::executeCycle(const std::vector<int>& Memory) {
    if (pipeline.back().valid) {
        RSEntry* parent = pipeline.back().parent;
        bool exception = false;
        int result = 0;
        parent->A = parent->Vj + parent->A; 
        if (parent->A < 0 or parent->A >= Memory.size()) {
            exception = true;
        } 
        else {
            if (parent->op == OpCode::LW) {
                result = Memory[parent->A];
                for (auto &entry : lsq) {
                    if (&entry == parent) break; 
                    if (entry.op == OpCode::SW && entry.A == parent->A) {
                        result = entry.Vk; 
                    }
                }
            }
        }
        ready_to_broadcast.push_back({ parent->rob_tag, result, true, exception });
    }
    for (int i = latency - 1; i > 0; i--) {
        pipeline[i] = pipeline[i - 1];
    }

    pipeline[0].valid = false;
    pipeline[0].parent = nullptr;
}

