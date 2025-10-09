#include "engine.h"
#include "transpositionTable.h"

using namespace std;

Engine::Engine(Board b) {
    board = b;
    bestMove = Move();
    TT = new TranspositionTable();
}

Engine::~Engine() {
    delete TT;
}

void Engine::resetEngine(Board b) {
    board = b;
    normalNodesSearched = 0;
    quiescenceNodesSearched = 0;
    tableAccesses = 0;
    tableAccessesQuiescence = 0;
    searchFinished = true;
    boardEval = 0;
    bestMove = Move(); 
    TT->clear();
}

void Engine::findBestMove(int t) {
    normalNodesSearched = 0;
    quiescenceNodesSearched = 0;
    tableAccesses = 0;
    tableAccessesQuiescence = 0;
    timeLimit = t;
    TT->incrementGeneration();

    int16_t turn = board.turn ? 1 : -1;

    cout << "Calculating best move... " << endl;
    
    auto start = chrono::high_resolution_clock::now();

    // Search
    startTime = chrono::high_resolution_clock::now();

    int d = 1;

    while (d <= MAX_DEPTH) {
        searchFinished = false;

        searchDepth = d;
        negaMax(d, -MATE, MATE, turn);

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

    cout << "Table Accesses Normal: " << tableAccesses << endl;
    cout << "Hit Rate Normal: " << (double) tableAccesses / (normalNodesSearched) * 100 << "%" << endl;
    cout << "Table Accesses Quiescence: " << tableAccessesQuiescence << endl;
    cout << "Hit Rate Quiescence: " << (double) tableAccessesQuiescence / quiescenceNodesSearched * 100 << "%" << endl;

    cout << endl;
}

int16_t Engine::negaMax(int depth, int16_t alpha, int16_t beta, int16_t turn) {
    if (depth == 0) {
        return quiescenceSearch(alpha, beta, turn); 
    }

    normalNodesSearched++;

    int16_t searchBestEval = -MATE;
    Move searchBestMove = Move();   // Set to default move

    // Generate posible moves
    MoveGen mg;
    mg.generatePseudoMoves(board);

    TTEntry entry;
    bool entryExists = TT->retrieveEntry(board.zobristHash, entry);
    uint16_t bestMoveValue = 0;

    if (entryExists) {
        bestMoveValue = entry.bestMove;

        // we have encountered this position before, don't search further.
        if (depth <= entry.depth) {
            if (entry.flag == EXACT) {
                tableAccesses++;
                searchBestMove = Move(entry.bestMove);
                searchBestEval = entry.evaluation;

                // Update class if correct depth
                if (depth == searchDepth) {
                    bestMove = searchBestMove;
                    boardEval = searchBestEval;
                    searchFinished = true;
                }

                return searchBestEval;
            }

        }
    }

    mg.orderMoves(board, bestMoveValue);

    int16_t alphaOrig = alpha;
    bool existsValidMove = false;
    for (Move move : mg.pseudoMoves) {
        if (isTimeUp()) {
            return -1; // Exit immediately with error value
        }
        
        board.makeMove(move);

        // Continue with valid positions
        if (!board.kingInCheck()) {
            // Evaluate child board from opponent POV
            int16_t eval = -negaMax(depth - 1, -beta, -alpha, -turn);

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
        } else {
            // this means checkmate. punish checkmates that occur sooner.
            searchBestEval = -MATE + (searchDepth - depth);
        }
    }

    uint8_t flag = EXACT;
    if (searchBestEval < alphaOrig) {
        flag = UPPERBOUND;
    } else if (searchBestEval > beta) {
        flag = LOWERBOUND;
    }

    if (!isTimeUp())  {
        TT->addEntry(board.zobristHash, searchBestMove.moveValue, searchBestEval, depth, flag);
    }

    // Update class if correct depth
    if (depth == searchDepth) {
        bestMove = searchBestMove;
        boardEval = searchBestEval;
        searchFinished = true;
    }

    return searchBestEval;
}

int16_t Engine::quiescenceSearch(int16_t alpha, int16_t beta, int16_t turn) {
    quiescenceNodesSearched++;

    int16_t bestSoFar = staticEvaluation(board) * turn;

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

    TTEntry entry;
    bool entryExists = TT->retrieveEntry(board.zobristHash, entry);
    uint16_t bestMoveValue = 0;
    if (entryExists) {
        bestMoveValue = entry.bestMove;

        if (entry.depth == 0 && entry.flag == EXACT) {
            tableAccessesQuiescence++;
            return entry.evaluation; // all depths stored will be better than the quiescence search
        }
    }

    mg.orderMoves(board, bestMoveValue); // only helps when the best move is a forcing move.

    int16_t alphaOrig = alpha;
    for (Move move : mg.pseudoMoves) {
        if (isTimeUp()) {
            return -1; // Exit immediately with error value
        }
        
        board.makeMove(move);

        // Continue with valid positions
        if (!board.kingInCheck()) {
            // Evaluate child board from opponent POV
            int16_t eval = -quiescenceSearch(-beta, -alpha, -turn);

            if (eval >= beta) {
                board.unmakeMove(move);
                return eval;
            }

            if (eval > bestSoFar) {
                bestMoveValue = move.moveValue;
                bestSoFar = eval;
            }

            alpha = max(alpha, bestSoFar);
        }
        
        board.unmakeMove(move);
    }

    uint8_t flag = EXACT;
    if (bestSoFar < alphaOrig) {
        flag = UPPERBOUND;
    } else if (bestSoFar > beta) {
        flag = LOWERBOUND;
    }

    if (!isTimeUp())  {
        TT->addEntry(board.zobristHash, bestMoveValue, bestSoFar, 0, flag);
    }

    return bestSoFar;
}