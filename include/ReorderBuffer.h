#pragma once
#include "Basics.h"
#include "CommonDataBus.h"

class ReorderBuffer {
public:
	std::vector<ROBEntry> buffer;
	int rob_size;
	int left = 0;
	int right = 0;
	int sz = 0;

	int getSize() const;
	bool isFull() const;
	bool isEmpty() const;
	int getNextTag() const;

	void flush();
	void insert(const ROBEntry new_entry);
	void remove();
	void listen(const CDB &bus);

};