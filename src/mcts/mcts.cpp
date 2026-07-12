#include "mcts.h"

MCTS::MCTS(Board b) : Agent(b) {}

MCTS::~MCTS() {}

// TODO: replace with a real MCTS search. Placeholder picks the first legal move.
void MCTS::findBestMove(int softLimit, int hardLimit, int maxDepth) {
    (void)softLimit; (void)hardLimit; (void)maxDepth;

    bestMove = Move();

    MoveGen& mg = MoveGen::getInstance();
    MoveList moves = mg.generateLegalMoves(board);
    if (moves.count > 0) {
        bestMove = moves.moves[0];
    }
    mg.freeLegalMoves(moves);
}

void MCTS::setPosition(Board b) {
    board = b;
}

void MCTS::reset(Board b) {
    board = b;
    bestMove = Move();
}

// TODO: implement simulation-based evaluation.
float MCTS::monteCarloTreeSearch(int numTrials) {
    (void)numTrials;
    return 0.0f;
}
