#pragma once
#include "Basics.h"

class CDB {
public:
    Broadcast current;
    void broadcast(int tag, int value, bool exception);
    void clear();
};