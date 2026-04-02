#include "../include/CommonDataBus.h"

void CDB::broadcast(int tag, int value, bool exception) {
    current = {tag, value, true, exception};
}

void CDB::clear() {
    current.valid = false;
}