#pragma once

#include <iostream>
#include <cstdint>
#include <random>

#include "chess/board/board.h"
#include "chess/moveGeneration/moveGen.h"
#include "engine/engine.h"

void displayBitboard(uint64_t bb);

void displayPossibleMoves(std::string FEN);
void simulateRandomMoves();

void perft(int depth, std::string FEN);
long perftRecursive(Board& b, int depth);

void playAI();
Move notationToMove(std::string move);