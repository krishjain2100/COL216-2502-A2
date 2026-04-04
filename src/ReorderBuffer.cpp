#include "../include/ReorderBuffer.h"

int ReorderBuffer::getSize() const {
	return sz;
}
bool ReorderBuffer::isFull() const {
	return (sz == rob_size);
}
bool ReorderBuffer::isEmpty() const {
	return !sz;
}
int ReorderBuffer::getNextTag() const {
	return tail;
}

void ReorderBuffer::flush() {
	head = tail = sz = 0;
	for(auto &e : buffer) e.valid = false;
}

void ReorderBuffer::insert(const ROBEntry new_entry) {
	if(isFull()) return;
	sz++;
	buffer[tail] = new_entry;
	tail = (tail +  1) % rob_size;
}

void ReorderBuffer::remove() {
	if(isEmpty()) return;
	if(buffer[head].valid) sz--;
	buffer[head].valid = false;
	buffer[head].busy = false;
	head = (head + 1) % rob_size;
	return;
}

void ReorderBuffer::listen(const CDB &bus) {
	if (!bus.current.valid) return;
	int tag = bus.current.tag;
	buffer[tag].value = bus.current.value;
	buffer[tag].busy = false;
	buffer[tag].exception = bus.current.exception;
}
