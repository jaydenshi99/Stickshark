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
    leafNodesEvaluated = 0;
    tableAccesses = 0;
    timeLimit = t;

    int turn = board.turn ? 1 : -1;

    cout << "Calculating best move... " << endl;
    
    auto start = chrono::high_resolution_clock::now();

    // Search
    startTime = chrono::high_resolution_clock::now();

    int d = 1;

    while (d <= MAX_DEPTH) {
        searchFinished = false;

        searchDepth = d;
        negaMax(d, MIN_EVAL, MAX_EVAL, turn);

        if (searchFinished) {
            d += 1;
        } else {
            searchDepth--;
            break;
        }
    }

    auto end = chrono::high_resolution_clock::now();

    // Debug
    cout << "Best move: " << bestMove << endl;
    cout << fixed << setprecision(2);
    cout << "Evaluation: " << (board.turn ? (float) boardEval / 100 : (float) -boardEval / 100) << endl << endl;

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << fixed << setprecision(0);
    cout << "Time taken: " << duration.count() << " ms" << endl;
    cout << "Depth searched: " << searchDepth << endl;
    cout << "Total nodes evaluated: " << leafNodesEvaluated << endl;

    double nodesPerSecond = leafNodesEvaluated / (static_cast<double>(duration.count()) / 1000);

    cout << fixed << setprecision(0);
    cout << "Nodes / Second: " << nodesPerSecond << endl;

    double branchingFactor = pow(static_cast<double>(leafNodesEvaluated), 1.0 / searchDepth);
    cout << fixed << setprecision(2);
    cout << "Branching factor: " << branchingFactor << endl;

    cout << "Table Accesses: " << tableAccesses << endl;

    cout << endl;
}

int Engine::negaMax(int depth, int alpha, int beta, int turn) {
    if (depth == 0) {
        leafNodesEvaluated++;
        return staticEvaluation(board) * turn; 
    }

    int searchBestEval = MIN_EVAL;
    Move searchBestMove = Move();   // Set to default move

    // Generate posible moves
    MoveGen mg;
    mg.generatePseudoMoves(board);

    uint16_t bestMoveValue = 0;
    if (retrieveBestMove(board.zobristHash, bestMoveValue)) {
        tableAccesses++;
    }

    mg.orderMoves(board, bestMoveValue);

    for (Move move : mg.moves) {
        if (isTimeUp()) {
            return -1; // Exit immediately with error value
        }
        
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

    storeBestMove(board.zobristHash, searchBestMove.moveValue);

    // Update class if correct depth
    if (depth == searchDepth) {
        bestMove = searchBestMove;
        boardEval = searchBestEval;
        searchFinished = true;
    }

    return searchBestEval;
}

void Engine::storeBestMove(uint64_t zobristKey, uint16_t moveValue) {
    if (bestMoveTable.size() >= BEST_MOVE_TABLE_MAX_SIZE) {
        bestMoveTable.erase(bestMoveTable.begin()); // Evict the first inserted entry
    }

    bestMoveTable[zobristKey] = moveValue;
}

bool Engine::retrieveBestMove(uint64_t zobristKey, uint16_t& moveValue) const {
    auto it = bestMoveTable.find(zobristKey); 

    if (it != bestMoveTable.end()) {
        moveValue = it->second; // Retrieve the associated moveValue
        return true;            // Indicate success
    }
    return false;               // Key not found
}