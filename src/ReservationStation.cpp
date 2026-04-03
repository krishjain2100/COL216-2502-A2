#include "../include/ReservationStation.h"


ReservationStation::ReservationStation(int rs_size) {
    entries.resize(rs_size);
}

int ReservationStation::getSize() const {
    return sz;
}
bool ReservationStation::isFull() const {
    int rs_size = entries.size();
    return (sz == rs_size);
}
bool ReservationStation::isEmpty() const {
    return !sz;
}

void ReservationStation::insert(const RSEntry &entry) {
    if(isFull()) return;
    for(auto &e : entries) {
        if(e.busy) continue;
        e = entry;
        break;
    }
    sz++;
}

void ReservationStation::clear() {
    for(auto &e: entries) e.busy = false;
    sz = 0;
}

void ReservationStation::listen(const CDB& bus) {
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

