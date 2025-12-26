#include "engine.h"
#include "transpositionTable.h"
#include <iomanip>

using namespace std;

Engine::Engine(Board b) {
    board = b;
    bestMove = Move();
    TT = new TranspositionTable();
    
    // Initialize all member variables
    searchDepth = 0;
    normalNodesSearched = 0;
    quiescenceNodesSearched = 0;
    tableProbes = 0;
    tableProbesQuiescence = 0;
    tableUsefulHits = 0;
    tableUsefulHitsQuiescence = 0;
    timeLimit = 1000;
    searchFinished = true;
    boardEval = 0;
    principalVariation.clear();
}

Engine::~Engine() {
    delete TT;
}

void Engine::resetEngine(Board b) {
    board = b;
    resetSearchStats();
    searchFinished = true;
    boardEval = 0;
    bestMove = Move();
    TT->clear();
}

void Engine::setPosition(Board b) {
    board = b;
    resetSearchStats();
    // Note: TT is NOT cleared - this preserves transposition table across position changes
}

void Engine::setUciInfoCallback(std::function<void(int depth, int timeMs, int nodes, int nps, int scoreCp, const std::vector<Move>& pv)> callback) {
    uciInfoCallback = callback;
}

void Engine::resetSearchStats() {
    normalNodesSearched = 0;
    quiescenceNodesSearched = 0;
    tableProbes = 0;
    tableProbesQuiescence = 0;
    tableUsefulHits = 0;
    tableUsefulHitsQuiescence = 0;
    principalVariation.clear();
}

void Engine::findBestMove(int t) {
    resetSearchStats();
    searchFinished = false;
    boardEval = 0;
    bestMove = Move();
    timeLimit = t;
    TT->incrementGeneration();

    int16_t turn = board.turn ? 1 : -1;

    cout << "Calculating best move... " << endl;
    
    auto start = chrono::steady_clock::now();

    // Search
    startTime = chrono::steady_clock::now();

    searchDepth = 1;

    while (searchDepth <= MAX_DEPTH) {
        searchFinished = false;
        negaMax(searchDepth, 0, -MATE, MATE, turn, true);

        cout << "Depth: " << searchDepth << " | Best move: " << bestMove << " | eval: " << boardEval << endl;

        if (searchFinished) {
            // Report UCI info for completed depth
            if (uciInfoCallback) {
                auto currentTime = chrono::steady_clock::now();
                auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(currentTime - startTime).count();
                int totalNodes = normalNodesSearched + quiescenceNodesSearched;
                int nps = elapsedTime > 0 ? (totalNodes * 1000) / elapsedTime : 0;
                
                // Convert score to side-to-move perspective
                int scoreCp = board.turn ? boardEval : -boardEval;
                
                uciInfoCallback(searchDepth, elapsedTime, totalNodes, nps, scoreCp, principalVariation);
            }
            searchDepth += 1;
        } else {
            searchDepth -= 1;
            break;
        }
    }

    auto end = chrono::steady_clock::now();

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

    cout << fixed << setprecision(2);

    cout << "Table Capacity: " << (float) TT->getNumFilledEntries() / (1 << 24) * 100 << "%" << endl;

    cout << "Probe Hit Rate Normal: " << (double) tableProbes / (normalNodesSearched) * 100 << "%" << endl;
    cout << "Useful Hit Rate Normal: " << (double) tableUsefulHits / (normalNodesSearched) * 100 << "%" << endl;
    cout << "Probe Hit Rate Quiescence: " << (double) tableProbesQuiescence / quiescenceNodesSearched * 100 << "%" << endl;
    cout << "Useful Hit Rate Quiescence: " << (double) tableUsefulHitsQuiescence / quiescenceNodesSearched * 100 << "%" << endl;

    cout << endl;
}

int16_t Engine::negaMax(int depth, int ply, int16_t alpha, int16_t beta, int16_t turn, bool isRoot) {
    if (isTimeUp()) {
        return 7777;
    }

    // Check for threefold
    if (board.isThreeFoldRepetition()) {
        return 0;
    }

    normalNodesSearched++;

    // Initialise variables
    int16_t searchBestEval = -MATE;
    Move searchBestMove = Move();   // Set to default move
    uint16_t bestMoveValue = 0;
    int16_t oldAlpha = alpha;
    int16_t oldBeta = beta;

    // Transposition Table Query
    TTEntry entry;
    bool entryExists = TT->retrieveEntry(board.zobristHash, entry);

    if (entryExists) {
        tableProbes++;  // Count every probe that finds an entry
        Move storedBestMove = Move(entry.bestMove);

        // test stored move for threefold repetition
        bool isThreeFoldRepetition = false;
        if (entry.bestMove != 0) {
            board.makeMove(storedBestMove);
            isThreeFoldRepetition = board.isThreeFoldRepetition();
            board.unmakeMove(storedBestMove);
        }

        if (!isThreeFoldRepetition) {
            bestMoveValue = entry.bestMove;

            if (entry.depth >= depth) {
                tableUsefulHits++;  // Count hits that can be used for cutoffs

                int16_t unpackedScore = entry.score;
                if (abs(entry.score) == MATE) {
                    if (entry.score > 0) {
                        unpackedScore = MATE - entry.plyToMate;
                    } else {
                        unpackedScore = -MATE + entry.plyToMate;
                    }
                }

                if (entry.flag == EXACT) {
                    searchBestEval = unpackedScore;
                    searchBestMove = storedBestMove;

                    if (isRoot) {
                        setFinalResult(searchBestEval, searchBestMove);
                    }

                    return searchBestEval;
                } else if (entry.flag == LOWERBOUND) {
                    if (unpackedScore >= beta) return beta;
                    alpha = max(alpha, unpackedScore);
                } else if (entry.flag == UPPERBOUND) {
                    if (unpackedScore <= alpha) return alpha;
                    beta = min(beta, unpackedScore);
                }

                if (alpha >= beta) return alpha;
            }
        }
    }
    
    // Quiescence search at leaf nodes
    if (depth == 0) {
        return quiescenceSearch(alpha, beta, turn); 
    }

    // Null move pruning guards, no checks + has pieces    
    uint64_t pieces = board.turn ? 
    board.pieceBitboards[WBISHOP] | board.pieceBitboards[WKNIGHT] | board.pieceBitboards[WROOK] | board.pieceBitboards[WQUEEN] :
    board.pieceBitboards[BBISHOP] | board.pieceBitboards[BKNIGHT] | board.pieceBitboards[BROOK] | board.pieceBitboards[BQUEEN];

    constexpr int r = 2; // reduced depth

    bool currentKingInCheck = board.kingInCheck(true);

    if (!currentKingInCheck && depth >= r + 1 && pieces != 0) {
        // Make the null move
        board.makeNullMove();

        // find null move eval by searching to reduced depth
        int16_t nullEval = -negaMax(depth - r - 1, ply + 1, -beta, -beta + 1, -turn); // no extensions for null move

        // unmake the null move
        board.unmakeNullMove();

        // if the null eval is greater than beta, we assume all moves will be greater than beta
        if (nullEval >= beta) {
            return nullEval;
        }
    }

    // Generate posible moves
    MoveGen& mg = MoveGen::getInstance();
    MoveList pseudoMoves = mg.generatePseudoMoves(board, false);
    mg.orderMoves(board, pseudoMoves, bestMoveValue);

    bool existsValidMove = false;
    bool first = true;
    for (std::ptrdiff_t i = 0; i < pseudoMoves.count; i++) {
        Move &move = pseudoMoves.moves[i];
        board.makeMove(move);

        // Continue with valid positions
        if (!board.kingInCheck(false)) {
            // Check extension with SEE guard (currently removed)
            int childDepth = depth - 1;
            if (board.kingInCheck(true)) {
                childDepth += 1;
            }

            // Evaluate child board from opponent POV
            int16_t score;
            if (first) {
                score = -negaMax(childDepth, ply + 1, -beta, -alpha, -turn);
                first = false;
            } else {
                // limited search to see if the next move beats our current one
                score = -negaMax(childDepth, ply + 1, -alpha - 1, -alpha, -turn);

                // if it does, we search again to full depth
                if (score > alpha && score < beta) {
                    score = -negaMax(childDepth, ply + 1, -beta, -alpha, -turn);
                }
            }

            if (isTimeUp()) {
                board.unmakeMove(move);
                mg.freePseudoMoves(pseudoMoves);
                return 7777;
            }

            // Update best evals and best moves
            if (score > searchBestEval) {
                searchBestEval = score;
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

    mg.freePseudoMoves(pseudoMoves);

    if (!existsValidMove) {
        if (!currentKingInCheck) {
            searchBestEval = 0;
        } else {
            // this means checkmate. Use ply to prefer shorter mates
            searchBestEval = -MATE + ply;
        }
    }

    // Update class if correct depth
    if (isRoot) {
        setFinalResult(searchBestEval, searchBestMove);
    }

    uint8_t flag = EXACT;
    if (searchBestEval <= oldAlpha) {
        flag = UPPERBOUND;
    } else if (searchBestEval >= oldBeta) {
        flag = LOWERBOUND;
    }

    TT->addEntry(board.zobristHash, searchBestMove.moveValue, searchBestEval, depth, flag);

    return searchBestEval;
}

int16_t Engine::quiescenceSearch(int16_t alpha, int16_t beta, int16_t turn) {
    if (isTimeUp()) {
        return 7777;
    }

    quiescenceNodesSearched++;

    // Check for threefold
    if (board.isThreeFoldRepetition()) {
        return 0;
    }

    int16_t bestSoFar = staticEvaluation(board) * turn;
    uint16_t bestMoveValue = 0;
    int16_t oldAlpha = alpha;
    int16_t oldBeta = beta;

    // Transposition Table Query
    TTEntry entry;
    bool entryExists = TT->retrieveEntry(board.zobristHash, entry);

    if (entryExists) {
        tableProbesQuiescence++;  // Count every probe that finds an entry
        Move storedBestMove = Move(entry.bestMove);

        bool isThreeFoldRepetition = false;
        if (entry.bestMove != 0) {
            board.makeMove(storedBestMove);
            isThreeFoldRepetition = board.isThreeFoldRepetition();
            board.unmakeMove(storedBestMove);
        }

        if (!isThreeFoldRepetition) {
            bestMoveValue = entry.bestMove;

            // unpack score: reconstruct mate score from stored distance
            int16_t unpackedScore = entry.score;
            if (abs(entry.score) == MATE) {
                // entry.plyToMate stores distance to mate (number of moves)
                if (entry.score > 0) {
                    unpackedScore = MATE - entry.plyToMate;
                } else {
                    unpackedScore = -MATE + entry.plyToMate;
                }
            }

            // Count as useful if it provides any value (exact, bound, or cutoff)
            bool useful = false;
            if (entry.flag == EXACT) {
                bestSoFar = unpackedScore;
                tableUsefulHitsQuiescence++;
                return bestSoFar;
            } else if (entry.flag == LOWERBOUND) {
                if (unpackedScore >= beta) {
                    tableUsefulHitsQuiescence++;
                    return beta;
                }
                alpha = max(alpha, unpackedScore);
                useful = true;
            } else if (entry.flag == UPPERBOUND) {
                if (unpackedScore <= alpha) {
                    tableUsefulHitsQuiescence++;
                    return alpha;
                }
                beta = min(beta, unpackedScore);
                useful = true;
            }

            if (alpha >= beta) {
                if (!useful) tableUsefulHitsQuiescence++;
                return alpha;
            } else if (useful) {
                tableUsefulHitsQuiescence++;
            }
        }
    }

    bool currKingInCheck = board.kingInCheck(true);

    // Standing Pat for non-check positions
    if (!currKingInCheck) {
        if (bestSoFar >= beta) {
            return bestSoFar;
        }
    
        alpha = max(alpha, bestSoFar);
    }

    // Generate forcing moves
    MoveGen& mg = MoveGen::getInstance();
    MoveList pseudoMoves = mg.generatePseudoMoves(board, !currKingInCheck); // force generating if own king is not in check. otherwise evasive moves
    mg.orderMoves(board, pseudoMoves, bestMoveValue); // only helps when the best move is a forcing move.

    for (std::ptrdiff_t i = 0; i < pseudoMoves.count; i++) {
        Move &move = pseudoMoves.moves[i];
        board.makeMove(move);

        // Continue with valid positions
        if (!board.kingInCheck(false)) {
            // Evaluate child board from opponent POV
            int16_t score = -quiescenceSearch(-beta, -alpha, -turn);
            if (isTimeUp()) {
                board.unmakeMove(move);
                mg.freePseudoMoves(pseudoMoves);
                return 7777;
            }

            if (score >= beta) {
                board.unmakeMove(move);
                mg.freePseudoMoves(pseudoMoves);
                return score;
            }

            if (score > bestSoFar) {
                bestMoveValue = move.moveValue;
                bestSoFar = score;
            }

            alpha = max(alpha, bestSoFar);
        }
        
        board.unmakeMove(move);
    }

    mg.freePseudoMoves(pseudoMoves);

    uint8_t flag = EXACT;
    if (bestSoFar <= oldAlpha) {
        flag = UPPERBOUND;
    } else if (bestSoFar >= oldBeta) {
        flag = LOWERBOUND;
    }

    TT->addEntry(board.zobristHash, bestMoveValue, bestSoFar, 0, flag);

    return bestSoFar;
}

void Engine::setFinalResult(int16_t score, Move& move) {
    bestMove = move;
    boardEval = score;
    searchFinished = true;
    
    // Update principal variation (simple version - just the best move)
    principalVariation.clear();
    if (move.getSource() != 0 || move.getTarget() != 0) { // Valid move
        principalVariation.push_back(move);
    }
}