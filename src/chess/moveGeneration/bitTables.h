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

#include "../../bit.h"
#include "../../engine/evalConstants.h"

#define NUM_FILES 8 
#define NUM_RANKS 8
#define NUM_SQUARES 64

#define BISHOP_ATTACKS_PER_SQUARE 512
#define ROOK_ATTACKS_PER_SQUARE 4096
#define BISHOP_ATTACK_TABLE_SIZE 32768
#define ROOK_ATTACK_TABLE_SIZE 262144

extern const bool isNonSliding[12];
extern const bool isPromotion[8];

extern const uint64_t fileBitboards[NUM_FILES];
extern const uint64_t rankBitboards[NUM_RANKS];

extern uint64_t notFileBitboards[NUM_FILES];
extern uint64_t notRankBitboards[NUM_RANKS];

extern uint64_t knightAttackBitboards[NUM_SQUARES];

extern uint64_t bishopAttackBitboards[BISHOP_ATTACK_TABLE_SIZE];
extern uint64_t rookAttackBitboards[ROOK_ATTACK_TABLE_SIZE];

extern uint64_t kingAttackBitboards[NUM_SQUARES];

extern uint64_t bishopAttackMagicMasks[NUM_SQUARES];
extern uint64_t rookAttackMagicMasks[NUM_SQUARES];

extern uint64_t bishopMagics[NUM_SQUARES];
extern uint64_t rookMagics[NUM_SQUARES];

// 0 - 63: WPAWN, 64 - 127: WBISHOP ...
extern uint64_t zobristBitstrings[773]; // 64 * 12 + 1 + 4 for pieces on squares and turn and castling rights


void computeAllTables();

void computeNotBitboards();

void computeKnightAttacks();

void computeSlidingAttacks();
uint64_t computeBishopAttackBitboard(int square, uint64_t blockers);
uint64_t computeRookAttackBitboard(int square, uint64_t blockers);

void computeKingAttacks();

// Magics
void computeMagics();

void computeMagicAttackMasks();
uint64_t generateRandomMagic();
bool testMagic(uint64_t magic, uint64_t mask, bool isBishop);
std::vector<uint64_t> generateAllOccupancies(uint64_t mask);

void loadMagics();
void saveMagics();

// Zobrist
void computeZobristBitstrings();