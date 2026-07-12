#include "testRunner.h"
#include "agent/agent.h"
#include "engine/engine.h"
#include "mcts/mcts.h"
#include "chess/board/board.h"
#include "chess/moveGeneration/moveGen.h"
#include "constants.h"
#include <memory>

using namespace std;

bool isLegalMove(Board& board, const Move& move) {
    MoveGen& mg = MoveGen::getInstance();
    MoveList legal = mg.generateLegalMoves(board);
    bool found = false;
    for (ptrdiff_t i = 0; i < legal.count; i++) {
        if (legal.moves[i].moveValue == move.moveValue) {
            found = true;
            break;
        }
    }
    mg.freeLegalMoves(legal);
    return found;
}

// Each agent, driven through the Agent base pointer, must produce a legal move.
bool testAgentsFindLegalMove() {
    Board board;
    board.setFEN(RANDOM_OPENING);

    unique_ptr<Agent> agents[] = {
        make_unique<Engine>(board),
        make_unique<MCTS>(board)
    };

    for (auto& agent : agents) {
        agent->findBestMove(200, 200, 4);
        ASSERT_TRUE(isLegalMove(agent->board, agent->bestMove));
    }

    return true;
}

// setPosition and reset must swap in the new position through the base interface.
bool testAgentSetPositionAndReset() {
    Board start;
    start.setFEN(STARTING_FEN);

    Board other;
    other.setFEN(ROOK_V_KING);

    unique_ptr<Agent> agents[] = {
        make_unique<Engine>(start),
        make_unique<MCTS>(start)
    };

    for (auto& agent : agents) {
        agent->setPosition(other);
        ASSERT_EQ(agent->board.zobristHash, other.zobristHash);

        agent->reset(start);
        ASSERT_EQ(agent->board.zobristHash, start.zobristHash);
    }

    return true;
}

int main() {
    computeAllTables();

    TestRunner runner;
    runner.run("Agents find a legal move via base interface", testAgentsFindLegalMove);
    runner.run("Agent setPosition and reset", testAgentSetPositionAndReset);
    runner.printSummary();
    return runner.getExitCode();
}
