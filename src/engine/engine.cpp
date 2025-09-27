#include "engine.h"

using namespace std;

Engine::Engine(Board b) {
    board = b;
    bestMove = Move();
}

void Engine::resetEngine(Board b) {
    board = b;
    normalNodesSearched = 0;
    quiescenceNodesSearched = 0;
    tableAccesses = 0;
    searchFinished = true;
    bestMoveTable.clear();
    boardEval = 0;
    bestMove = Move(); 
}

void Engine::findBestMove(int t) {
    normalNodesSearched = 0;
    quiescenceNodesSearched = 0;
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
    cout << endl << "Best move: " << bestMove << endl;
    cout << fixed << setprecision(2);
    cout << "Evaluation: " << (board.turn ? (float) boardEval / 100 : (float) -boardEval / 100) << endl << endl;

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << fixed << setprecision(0);
    cout << "Time taken: " << duration.count() << " ms" << endl;
    cout << "Depth searched: " << searchDepth << endl;
    cout << "Normal nodes searched: " << normalNodesSearched << endl;
    cout << "Quiescence nodes searched: " << quiescenceNodesSearched << endl;

    int nodesSearched = normalNodesSearched + quiescenceNodesSearched;
    cout << "Total nodes searched: " << nodesSearched << endl;
    cout << "Percentage quiescene: " << (double) quiescenceNodesSearched / nodesSearched * 100 << "%" << endl;

    double nodesPerSecond = nodesSearched / (static_cast<double>(duration.count()) / 1000);

    cout << fixed << setprecision(0);
    cout << "Nodes / Second: " << nodesPerSecond << endl;

    cout << endl;

    cout << "Table Accesses: " << tableAccesses << endl;

    cout << endl;
}

int Engine::negaMax(int depth, int alpha, int beta, int turn) {
    if (depth == 0) {
        //return staticEvaluation(board) * turn;
        return quiescenceSearch(alpha, beta, turn); 
    }

    normalNodesSearched++;

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

    bool existsValidMove = false;
    for (Move move : mg.pseudoMoves) {
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

            // Update existsValidMove
            existsValidMove = true;

            // Alpha-beta pruning
            alpha = max(alpha, searchBestEval);
            if (alpha >= beta) {
                board.unmakeMove(move);
                break;
            }
        }
        
        board.unmakeMove(move);
    }

    if (!existsValidMove) {
        bool currKingInCheck = board.pieceBitboards[board.turn ? WKING : BKING] & (board.turn ? board.getBlackAttacks() : board.getWhiteAttacks());
        if (!currKingInCheck) {
            searchBestEval = 0;
        }
    }

    if (existsValidMove && !isTimeUp()) storeBestMove(board.zobristHash, searchBestMove.moveValue);

    // Update class if correct depth
    if (depth == searchDepth) {
        bestMove = searchBestMove;
        boardEval = searchBestEval;
        searchFinished = true;
    }

    return searchBestEval;
}

int Engine::quiescenceSearch(int alpha, int beta, int turn) {
    quiescenceNodesSearched++;

    int bestSoFar = staticEvaluation(board) * turn;

    bool currKingInCheck = board.pieceBitboards[board.turn ? WKING : BKING] & (board.turn ? board.getBlackAttacks() : board.getWhiteAttacks());

    // Standing Pat for non-check positions
    if (!currKingInCheck) {
        if (bestSoFar >= beta) {
            return bestSoFar;
        }
    
        alpha = max(alpha, bestSoFar);
    }

    // Generate forcing moves
    MoveGen mg;
    mg.onlyGenerateForcing = !currKingInCheck; // force generating if own king is not in check. otherwise evasive moves
    mg.generatePseudoMoves(board);

    for (Move move : mg.pseudoMoves) {
        if (isTimeUp()) {
            return -1; // Exit immediately with error value
        }
        
        board.makeMove(move);

        // Continue with valid positions
        if (!board.kingInCheck()) {
            // Evaluate child board from opponent POV
            int eval = -quiescenceSearch(-beta, -alpha, -turn);

            if (eval >= beta) {
                board.unmakeMove(move);
                return eval;
            }

            if (eval > bestSoFar) {
                bestSoFar = eval;
            }

            alpha = max(alpha, bestSoFar);
        }
        
        board.unmakeMove(move);
    }

    return bestSoFar;
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