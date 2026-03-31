#pragma once
#include "Basics.h"


class ReorderBuffer {
public:
	std::vector<ROBEntry> buffer;
	int left = 0;
	int right = 0;
	int sz = 0;

	int getSize() const {
		return sz;
	}

	bool isFull() const {
		return (sz == rob_size);
	}

	bool isEmpty() const {
		return !sz;
	}

	int getNextTag() const {
		return right;
	}

	void flush() {
		left = 0;
		right = 0;
		sz = 0;
	}


	void insert(const ROBEntry new_entry) {
		if(isFull()) return;
		sz++;
		buffer[right] = new_entry;
		right = (right +  1) % rob_size;
	}

	void remove() {
		if(isEmpty()) return;
		sz--;
		buffer[left].busy = false;
		left = (left + 1) % rob_size;
		return;
	}

	void listen(const CDB &bus) {
		if (!bus.current.valid) return;
		int tag = bus.current.tag;
		buffer[tag].value = bus.current.value;
		buffer[tag].busy = false;
		buffer[tag].exception = bus.current.exception;
	}

};