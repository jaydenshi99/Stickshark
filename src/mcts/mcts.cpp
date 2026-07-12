#include "mcts.h"

MCTS::MCTS(Board b) : Agent(b) {
    net.load("data/model.nn");
    nnPlanes.resize(nn::PLANES * 64);
    nnLogits.resize(nn::POLICY_SIZE);
}

MCTS::~MCTS() {}

void MCTS::findBestMove(int softLimit, int hardLimit, int maxDepth) {
    arena.clear();
    arena.push_back(Node{});
    bestMove = Move();

    // fixed for now, make dynamic later
    const int trials = net.isLoaded() ? 800 : 100000;

    auto start = chrono::steady_clock::now();
    monteCarloTreeSearch(trials);
    int timeMs = (int)chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count();

    const Node& root = arena[0];
    uint32_t maxVisits = 0;
    uint32_t bestIdx = 0;
    for (uint32_t ci = root.firstChild; ci < root.firstChild + root.numChildren; ++ci) {
        if (arena[ci].visits > maxVisits) {
            maxVisits = arena[ci].visits;
            bestIdx = ci;
            bestMove = Move(arena[ci].move);
        }
    }

    // Report the Q of the move actually picked
    if (bestIdx != 0 && uciInfoCallback) {
        float q = -arena[bestIdx].valueSum / (float)arena[bestIdx].visits;
        q = clamp(q, -0.9999f, 0.9999f);
        int cp = (int)(atanh(q) * 300.0f);
        int scoreCp = board.turn ? cp : -cp; 
        int nps = timeMs > 0 ? (int)((int64_t)trials * 1000 / timeMs) : 0;
        vector<Move> pv{bestMove};
        uciInfoCallback(0, timeMs, trials, nps, scoreCp, pv);
    }

    reportMoveDistribution();
}

// Emit the root children as "movedist <uci>:<visits>:<q> ..." (descending by
// visits). q is from the root mover's perspective.
void MCTS::reportMoveDistribution() {
    const Node& root = arena[0];
    if (!infoStringCallback || root.numChildren == 0) return;

    vector<uint32_t> kids;
    for (uint32_t ci = root.firstChild; ci < root.firstChild + root.numChildren; ++ci) {
        if (arena[ci].visits > 0) {
            kids.push_back(ci);
        }
    }
    sort(kids.begin(), kids.end(), [this](uint32_t a, uint32_t b) { return arena[a].visits > arena[b].visits; });

    string s = "movedist";
    char qbuf[16];
    for (uint32_t ci : kids) {
        float q = -arena[ci].valueSum / (float)arena[ci].visits;
        snprintf(qbuf, sizeof(qbuf), "%.3f", q);
        s += " " + Move(arena[ci].move).toUci() + ":" + to_string(arena[ci].visits) + ":" + qbuf;
    }
    infoStringCallback(s);
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

        // 2+3. expand the leaf and evaluate it (NN policy+value if loaded);
        // terminal and repetition nodes use the exact result
        float value;
        if (arena[nIdx].firstChild != 0) {
            value = board.kingInCheck(true) ? -1.0f : 0.0f;   // mate : stalemate
        } else {
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

            if (arena[nIdx].numChildren == 0) {
                value = board.kingInCheck(true) ? -1.0f : 0.0f;
            } else {
                value = evaluateLeaf(nIdx);
                if (board.isThreeFoldRepetition()) {
                    value = 0.0f;
                }
            }
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

// Sets the priors on nodeIdx's children and returns the leaf value.
// Falls back to uniform priors + static eval when no weights are loaded.
float MCTS::evaluateLeaf(uint32_t nodeIdx) {
    uint32_t first = arena[nodeIdx].firstChild;
    uint16_t n = arena[nodeIdx].numChildren;

    if (!net.isLoaded()) {
        for (uint16_t i = 0; i < n; i++) {
            arena[first + i].prior = 1.0f / n;
        }
        int cp = staticEvaluation(board);
        if (!board.turn) cp = -cp;          // eval is white-POV; valueSum is side-to-move
        return tanh(cp / 300.0f);
    }

    nn::encodeBoard(board, nnPlanes.data());
    float value = net.evaluate(nnPlanes.data(), nnLogits.data());

    // softmax over the legal moves' logits
    float maxLogit = -1e30f;
    for (uint16_t i = 0; i < n; i++) {
        int idx = nn::policyIndex(Move(arena[first + i].move), board.turn);
        float logit = nnLogits[idx];
        arena[first + i].prior = logit;
        if (logit > maxLogit) maxLogit = logit;
    }
    float sum = 0.0f;
    for (uint16_t i = 0; i < n; i++) {
        float e = exp(arena[first + i].prior - maxLogit);
        arena[first + i].prior = e;
        sum += e;
    }
    for (uint16_t i = 0; i < n; i++) {
        arena[first + i].prior /= sum;
    }

    return value;
}

// PUCT
uint32_t MCTS::selectChild(uint32_t parentIdx) {
    const Node& p = arena[parentIdx];
    float sqrtN = sqrt((float)p.visits);

    uint32_t best = p.firstChild;
    float bestScore = -1e30f;
    for (uint32_t ci = p.firstChild; ci < p.firstChild + p.numChildren; ci++) {
        const Node& ch = arena[ci];
        float q = ch.visits > 0 ? -ch.valueSum / ch.visits : 0.0f;
        float score = q + C * ch.prior * sqrtN / (1.0f + ch.visits);
        if (score > bestScore) { bestScore = score; best = ci; }
    }
    return best;
}

