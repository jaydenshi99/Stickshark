#pragma once

#include <cstdint>

#define NUMFILES 8 
#define NUMRANKS 8

// Tables
extern const uint64_t fileBitboards[NUMFILES];
extern const uint64_t rankBitboards[NUMRANKS];

extern uint64_t notFileBitboards[NUMFILES];
extern uint64_t notRankBitboards[NUMRANKS];

// Functions
void computeAllTables();

void computeNotBitboards();