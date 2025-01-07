#include "engine.h"

using namespace std;

Engine::Engine(Board b) {
    board = b;
    bestMove = Move();
}

void Engine::setBoard(Board b) {
    board = b;
}

void Engine::findBestMove(int t) {
    searchDepth = 0;
    timeLimit = t;

    cout << "Calculating best move... " << endl;
    
    auto start = chrono::high_resolution_clock::now();

    // Search
    startTime = chrono::high_resolution_clock::now();

    int d = 0;
    int turn = board.turn ? 1 : -1;

    while (d <= MAX_DEPTH) {
        leafNodesEvaluated = 0;
        searchFinished = false;
        d++;

        searchDepth = d;

        negaMax(d, MIN_EVAL, MAX_EVAL, turn);

        if (!searchFinished) {
            searchDepth--;
            break;
        }
    }

    auto end = chrono::high_resolution_clock::now();

    // Debug
    cout << "Best move: " << bestMove << endl;
    cout << "Evaluation: " << (board.turn ? boardEval : -boardEval) << endl << endl;

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Time taken: " << duration.count() << " ms" << endl;
    cout << "Depth searched: " << searchDepth << endl;
    cout << "Nodes evaluated: " << leafNodesEvaluated << endl;

    double nodesPerSecond = leafNodesEvaluated / (static_cast<double>(duration.count()) / 1000);

    cout << fixed << setprecision(0);
    cout << "Nodes / Second: " << nodesPerSecond << endl;

    double branchingFactor = pow(static_cast<double>(leafNodesEvaluated), 1.0 / searchDepth);
    cout << fixed << setprecision(2);
    cout << "Branching factor: " << branchingFactor << endl;

    cout << endl;
}

int Engine::negaMax(int depth, int alpha, int beta, int turn) {
    if (depth == 0) {
        leafNodesEvaluated++;
        return evaluateBoard(board) * turn; // 
    }

    int searchBestEval = MIN_EVAL;
    Move searchBestMove = Move();   // Set to default move

    // Generate posible moves
    MoveGen mg;
    mg.generatePseudoMoves(board);
    mg.orderMoves(board);

    for (Move move : mg.moves) {
        board.makeMove(move);

        // Continue with valid positions
        if (!board.kingInCheck()) {
            // Evaluate child board from opponent POV
            int eval = -negaMax(depth - 1, -beta, -alpha, -turn);

            // Update best evals and best moves
            if (eval > searchBestEval) {
                searchBestEval = eval;
                searchBestMove = move;
            }

            // Alpha-beta pruning
            alpha = max(alpha, searchBestEval);
            if (alpha >= beta) {
                board.unmakeMove(move);
                break;
            }
        }
        
        board.unmakeMove(move);
    }

    if (isTimeUp()) {
        return -1; // Garbage value
    }

    // Update class if correct depth
    if (depth == searchDepth) {
        bestMove = searchBestMove;
        boardEval = searchBestEval;
        searchFinished = true;
    }

    return searchBestEval;
}