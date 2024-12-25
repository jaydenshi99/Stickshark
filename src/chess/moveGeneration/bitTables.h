#pragma once

#include <cstdint>
#include <vector>
#include <bit>
#include <cmath>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>

#include "../bit.h"

#define NUM_FILES 8 
#define NUM_RANKS 8
#define NUM_SQUARES 64

#define BISHOP_MOVE_TABLE_SIZE 32768
#define ROOK_MOVE_TABLE_SIZE 262144


extern const uint64_t fileBitboards[NUM_FILES];
extern const uint64_t rankBitboards[NUM_RANKS];

extern uint64_t notFileBitboards[NUM_FILES];
extern uint64_t notRankBitboards[NUM_RANKS];

extern uint64_t knightAttackBitboards[NUM_SQUARES];

extern uint64_t bishopAttackMagicMasks[NUM_SQUARES];
extern uint64_t rookAttackMagicMasks[NUM_SQUARES];

extern uint64_t bishopMagics[NUM_SQUARES];
extern uint64_t rookMagics[NUM_SQUARES];


void computeAllTables();

void computeNotBitboards();

void computeKnightAttacks();

// Magics
void computeMagics();

void computeMagicAttackMasks();
uint64_t generateRandomMagic();
bool testMagic(uint64_t magic, uint64_t mask, bool isBishop);
std::vector<uint64_t> generateAllOccupancies(uint64_t mask);

void loadMagics();
void saveMagics();