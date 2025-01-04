#include "engine.h"

Engine::Engine(Board b) {
    board = b;
    bestMove = Move(0, 0, 0);
    moveEval = 0;
}