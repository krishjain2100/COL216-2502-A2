#include "../include/LoadStoreQueue.h"

LoadStoreQueue::LoadStoreQueue() {}

LoadStoreQueue::LoadStoreQueue(int _latency, int _rs_size) : latency(_latency), rs_size(_rs_size) {
	entries.resize(_rs_size);
	pipeline.resize(_latency);
}

int LoadStoreQueue::getSize() const {
	return sz;
}

bool LoadStoreQueue::isFull() const {
	return sz >= rs_size;
}

bool LoadStoreQueue::push(const RSEntry &entry) {
	if (isFull()) return false;
	entries[tail] = entry;
	tail = (tail + 1) % rs_size;
	sz++;
	return true;
}

void LoadStoreQueue::listen(const CDB& bus) {
	if (!bus.current.valid or sz == 0) return;
	int i = head;
	for (int count = 0; count < sz; count++) {
		if (entries[i].Qj == bus.current.tag) {
			entries[i].Vj = bus.current.value;
			entries[i].Qj = -1;
		}
		if (entries[i].Qk == bus.current.tag) {
			entries[i].Vk = bus.current.value;
			entries[i].Qk = -1;
		}
		i = (i + 1) % rs_size;
	}
}

void LoadStoreQueue::flush() {
	for (auto& stage : pipeline) {
		stage.valid = false;
		stage.parent = nullptr;
	}
	for(auto &e : entries) e.valid = false;
	ready_to_broadcast.clear();
	head = tail = sz = 0;
}

void LoadStoreQueue::pop() {
	if (sz == 0) return;
	entries[head].valid = false;
	head = (head + 1) % rs_size;
	sz--;
}

void LoadStoreQueue::dispatch() {
	if (latency == 0 or pipeline[0].valid or sz == 0) return;
	int i = head;
	for (int count = 0; count < sz; count++) {
		if (!entries[i].executing) {
			if (entries[i].Qj == -1 && entries[i].Qk == -1) {
				entries[i].executing = true;
				pipeline[0].parent = &entries[i];
				pipeline[0].valid = true;
			}
			break; 
		}
		i = (i + 1) % rs_size;
	}
}

void LoadStoreQueue::executeCycle(const std::vector<int>& Memory) {
	if (latency == 0) return;

	if (pipeline.back().valid) {
		RSEntry* parent = pipeline.back().parent;
		bool exception = false;
		int result = 0;
		parent->A = parent->Vj + parent->A; 
		if (parent->A < 0 or parent->A >= (int)Memory.size()) {
			exception = true;
		} else {
			if (parent->op == OpCode::LW) {
				result = Memory[parent->A];
				int i = head;
				while (&entries[i] != parent) {
					if (entries[i].op == OpCode::SW && entries[i].A == parent->A) {
						result = entries[i].Vk;
					}
					i = (i + 1) % rs_size;
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
