#pragma once

#include <iostream>
#include <cstdint>
#include <random>

#include "./chess/board/board.h"
#include "./chess/moveGeneration/moveGen.h"

void displayBitboard(uint64_t bb);

void displayPossibleMoves();
void simulateRandomMoves();

void perft(int depth);
int perftRecursive(Board& b, int depth);