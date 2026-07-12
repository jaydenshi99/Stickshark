#include "mcts.h"

MCTS::MCTS(Board b) : Agent(b) {}

MCTS::~MCTS() {}

void MCTS::findBestMove(int softLimit, int hardLimit, int maxDepth) {
    arena.clear();
    arena.push_back(Node{});

    monteCarloTreeSearch(10000);

    const Node& root = arena[0];
    uint32_t maxVisits = 0;
    for (uint32_t ci = root.firstChild; ci < root.firstChild + root.numChildren; ++ci) {
        if (arena[ci].visits > maxVisits) {
            maxVisits = arena[ci].visits;
            bestMove = Move(arena[ci].move);
        }
    }
}

void MCTS::setPosition(Board b) {
    board = b;
}

void MCTS::reset(Board b) {
    board = b;
    bestMove = Move();
}

float MCTS::monteCarloTreeSearch(int numTrials) {

    for (int i = 0; i < numTrials; ++i) {
        // 1. descend via UCT until we reach an unexpanded or terminal node
        uint32_t nIdx = 0;
        path.clear();
        path.push_back(0);
        while (arena[nIdx].numChildren != 0) {
            nIdx = selectChild(nIdx);
            board.makeMove(Move(arena[nIdx].move));
            path.push_back(nIdx);
        }

        // 2. expand the leaf: append all legal moves as one contiguous child block
        if (arena[nIdx].firstChild == 0) {
            MoveGen& mg = MoveGen::getInstance();
            MoveList legal = mg.generateLegalMoves(board);

            uint32_t start = (uint32_t)arena.size();
            for (std::ptrdiff_t j = 0; j < legal.count; j++) {
                Node child;
                child.move = legal.moves[j].moveValue;
                arena.push_back(child);
            }
            arena[nIdx].firstChild = start;
            arena[nIdx].numChildren = (uint16_t)legal.count;

            mg.freeLegalMoves(legal);
        }

        // descend into one (unvisited) child so the rollout starts there
        if (arena[nIdx].numChildren > 0) {
            nIdx = selectChild(nIdx);
            board.makeMove(Move(arena[nIdx].move));
            path.push_back(nIdx);
        }
        // 3. rollout from the reached node; 
        float value;
        if (arena[nIdx].numChildren == 0 && arena[nIdx].firstChild != 0) {
            value = board.kingInCheck(true) ? -1.0f : 0.0f;   // mate : stalemate
        } else {
            value = rollout();
        }

        // 4. backprop, unmaking the path's moves on the way up
        for (int j = (int)path.size() - 1; j >= 0; j--) {
            Node& n = arena[path[j]];
            n.visits++;
            n.valueSum += value;
            value = -value;
            if (j > 0) board.unmakeMove(Move(n.move));
        }
    }

    return 0.0f;
}

// Play uniformly random legal moves from the current board position until the
// game ends or the ply cap is hit
float MCTS::rollout() {
    MoveGen& mg = MoveGen::getInstance();

    constexpr int ROLLOUT_CAP = 200;
    Move made[ROLLOUT_CAP];
    int madeCount = 0;

    float result = 0.0f;   
    int perspective = 1;   // +1 while side to move == side at rollout start

    while (madeCount < ROLLOUT_CAP) {
        if (board.isThreeFoldRepetition()) break;
        MoveList pseudo = mg.generatePseudoMoves(board, false);
        int remaining = (int)pseudo.count;
        bool moved = false;
        while (remaining > 0) {
            int k = (int)(nextRandom() % (uint64_t)remaining);
            Move m = pseudo.moves[k];
            board.makeMove(m);
            if (board.kingInCheck(false)) {
                board.unmakeMove(m);
                pseudo.moves[k] = pseudo.moves[remaining - 1];
                remaining--;
            } else {
                made[madeCount++] = m;
                moved = true;
                break;
            }
        }
        mg.freePseudoMoves(pseudo);

        if (!moved) {
            // No legal moves: mate or stalemate for the current side to move
            result = board.kingInCheck(true) ? (float)-perspective : 0.0f;
            break;
        }
        perspective = -perspective;
    }

    // Restore the board to the rollout's starting position
    for (int i = madeCount - 1; i >= 0; i--) {
        board.unmakeMove(made[i]);
    }

    return result;
}

uint32_t MCTS::selectChild(uint32_t parentIdx) {
    const Node& p = arena[parentIdx];
    float logN = std::log((float)p.visits);

    uint32_t best = 0;
    float bestScore = -1e30f;
    for (uint32_t ci = p.firstChild; ci < p.firstChild + p.numChildren; ci++) {
        const Node& ch = arena[ci];
        if (ch.visits == 0) return ci;
        float q = -ch.valueSum / ch.visits;
        float score = q + C * std::sqrt(logN / ch.visits);
        if (score > bestScore) { bestScore = score; best = ci; }
    }
    return best;
}

