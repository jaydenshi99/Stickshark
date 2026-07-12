#include "encoder.h"

#include <algorithm>

namespace nn {

static inline int orient(int sq, bool white) { return white ? sq : sq ^ 56; }

static void fillPlane(float* planes, int plane, float v) {
    std::fill(planes + plane * 64, planes + (plane + 1) * 64, v);
}

void encodeBoard(const Board& board, float* planes) {
    std::fill(planes, planes + PLANES * 64, 0.0f);
    bool white = board.turn;

    for (int sq = 0; sq < 64; sq++) {
        int piece = board.squares[sq];
        if (piece == EMPTY) continue;
        bool ours = (piece < BPAWN) == white;
        int plane = (ours ? 0 : 6) + piece % 6;
        planes[plane * 64 + orient(sq, white)] = 1.0f;
    }

    const Gamestate& gs = board.history.top();
    uint8_t cr = gs.castlingRights;
    if (cr & (white ? 1 : 4)) fillPlane(planes, 12, 1.0f);
    if (cr & (white ? 2 : 8)) fillPlane(planes, 13, 1.0f);
    if (cr & (white ? 4 : 1)) fillPlane(planes, 14, 1.0f);
    if (cr & (white ? 8 : 2)) fillPlane(planes, 15, 1.0f);

    if (gs.enpassantColumn != -1) {
        for (int r = 0; r < 8; r++) {
            planes[16 * 64 + r * 8 + gs.enpassantColumn] = 1.0f;
        }
    }

    if (board.isThreeFoldRepetition(1)) fillPlane(planes, 17, 1.0f);

    fillPlane(planes, 18, 1.0f);
}

int policyIndex(const Move& move, bool whiteToMove) {
    return orient(move.getSource(), whiteToMove) * 64 + orient(move.getTarget(), whiteToMove);
}

}
