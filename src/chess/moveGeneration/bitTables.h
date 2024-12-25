#pragma once

#include <cstdint>
#include <vector>
#include <bit>
#include <cmath>

#include "../bit.h"

#define NUMFILES 8 
#define NUMRANKS 8
#define NUMSQUARES 64

// Tables
extern const uint64_t fileBitboards[NUMFILES];
extern const uint64_t rankBitboards[NUMRANKS];

extern uint64_t notFileBitboards[NUMFILES];
extern uint64_t notRankBitboards[NUMRANKS];

extern uint64_t knightAttackBitboards[NUMSQUARES];

extern uint64_t bishopAttackMagicMasks[NUMSQUARES];
extern uint64_t rookAttackMagicMasks[NUMSQUARES];
extern uint64_t bishopMagics[NUMSQUARES];
extern uint64_t rookMagics[NUMSQUARES];

// Functions
void computeAllTables();

void computeNotBitboards();

void computeKnightAttacks();

void computeMagicAttackMasks();

void computeMagics();

std::vector<uint64_t> generateAllOccupancies(uint64_t mask);