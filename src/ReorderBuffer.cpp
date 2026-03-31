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
    return right;
}

void ReorderBuffer::flush() {
    left = 0;
    right = 0;
    sz = 0;
}

void ReorderBuffer::insert(const ROBEntry new_entry) {
    if(isFull()) return;
    sz++;
    buffer[right] = new_entry;
    right = (right +  1) % rob_size;
}

void ReorderBuffer::remove() {
    if(isEmpty()) return;
    sz--;
    buffer[left].busy = false;
    left = (left + 1) % rob_size;
    return;
}

void ReorderBuffer::listen(const CDB &bus) {
    if (!bus.current.valid) return;
    int tag = bus.current.tag;
    buffer[tag].value = bus.current.value;
    buffer[tag].busy = false;
    buffer[tag].exception = bus.current.exception;
}
