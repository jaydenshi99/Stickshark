#include "engine.h"

using namespace std;

Engine::Engine(Board b) {
    board = b;
    bestMove = Move(0, 0, 0);
    boardEval = 0;
    searchDepth = 0;
    leafNodesEvaluated = 0;
}

void Engine::setBoard(Board b) {
    board = b;
}

void Engine::findBestMove(int depth) {
    leafNodesEvaluated = 0;
    searchDepth = depth;

    cout << "Calculating best move for board: " << endl;
    board.displayBoard();

    
    auto start = chrono::high_resolution_clock::now();

    // Search
    negaMax(searchDepth);

    auto end = chrono::high_resolution_clock::now();
    
    cout << "Best move: " << endl;
    cout << "Evaluation: " << boardEval << endl;

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Time taken: " << duration.count() << " ms" << endl;
    cout << "Depth searched: " << searchDepth << endl;
    cout << "Nodes evaluated: " << leafNodesEvaluated << endl;

    double nodesPerSecond = leafNodesEvaluated / (static_cast<double>(duration.count()) / 1000);

    cout << fixed << setprecision(0);
    cout << "Nodes / Second: " << nodesPerSecond << endl;

    double branchingFactor = pow(static_cast<double>(leafNodesEvaluated), 1.0 / depth);
    cout << fixed << setprecision(2);
    cout << "Branching factor: " << branchingFactor << endl;

    cout << endl;
}

int Engine::negaMax(int depth) {
    // At 0 depth return 0 for now in future replace this with the evaluation function
    if (depth == 0) {
        leafNodesEvaluated++;
        return 0;
    }

    int searchBestEval = numeric_limits<int>::min();
    Move searchBestMove = Move();   // Set to default move

    // Generate posible moves
    MoveGen mg;
    mg.generatePseudoMoves(board);

    for (Move move : mg.moves) {
        board.makeMove(move);

        // Continue with valid positions
        if (!board.kingInCheck()) {
            // Evaluate child board from opponent POV
            int eval = -negaMax(depth - 1);

            // Update best evals and best moves
            if (eval > searchBestEval) {
                searchBestEval = eval;
                searchBestMove = move;
            }
        }
        
        board.unmakeMove(move);
    }

    // Update class if correct depth
    if (depth == searchDepth) {
        bestMove = searchBestMove;
        boardEval = searchBestEval;
    }

    return searchBestEval;
}