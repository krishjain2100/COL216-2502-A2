#pragma once
#include "Basics.h"

class ReservationStation {
public:
	int rs_size;
	int sz = 0;
	vector <RSEntry> entries;

	ReservationStation(int _rs_size) {
		rs_size = _rs_size;
		entries.resize(rs_size);
	}

	int getSize() const {
		return sz;
	}

	bool isFull() const {
		return (sz == rs_size);
	}

	bool isEmpty() const {
		return !sz;
	}

	void insert(RSEntry &entry) {
		if(isFull()) return;
		for(auto &e : entries) {
			if(!e.busy) {
				e = entry;
				break;
			}
		}
		sz++;
	}

	void remove(RSEntry &entry) {
		if(isEmpty()) return;
		bool found = false;
		for(auto &e : entries) {
			if(e == entry) {
				found = true;
				e.busy = false;
			}
		}
		if(found) sz--;
	}

	void clear() {
		for(auto &e: entries) {
			e.busy = false;
		}
	}


	void listen(const CDB& bus) {
		if (!bus.current.valid) return;
		for(auto &entry : entries) {
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

	void updateReady(vector<RSEntry> &ready_for_execution) {
		for(auto &entry : entries) {
			bool ready = (entry.Qj == -1) and (entry.Qk == -1);
			if(ready) {
				ready_for_execution.push_back(entry);
				entry.busy = false;
				sz--;
			}
		}
	}
};