#pragma once
#include <vector>
#include "Basics.h"
#include "CommonDataBus.h"

class ReservationStation {
public:
	int sz = 0;
	std::vector<RSEntry> entries;

	ReservationStation(int rs_size);

	int getSize() const;
	bool isFull() const;
	bool isEmpty() const;

	void insert(const RSEntry &entry);
	void clear();
	
	void listen(const CDB& bus);
};
		