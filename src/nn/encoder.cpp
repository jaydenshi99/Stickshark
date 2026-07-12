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

void encodeCompact(const Board& board, CompactPosition& out) {
    bool white = board.turn;

    // rank-mirroring a bitboard (sq ^ 56 per bit) is exactly a byte swap
    for (int t = 0; t < 6; t++) {
        uint64_t ours = board.pieceBitboards[white ? t : t + 6];
        uint64_t theirs = board.pieceBitboards[white ? t + 6 : t];
        out.bb[t] = white ? ours : __builtin_bswap64(ours);
        out.bb[t + 6] = white ? theirs : __builtin_bswap64(theirs);
    }

    const Gamestate& gs = board.history.top();
    uint8_t cr = gs.castlingRights;
    uint8_t f = 0;
    if (cr & (white ? 1 : 4)) f |= 1;
    if (cr & (white ? 2 : 8)) f |= 2;
    if (cr & (white ? 4 : 1)) f |= 4;
    if (cr & (white ? 8 : 2)) f |= 8;
    if (board.isThreeFoldRepetition(1)) f |= 16;
    out.flags = f;
    out.epColumn = gs.enpassantColumn == -1 ? 255 : (uint8_t)gs.enpassantColumn;
}

}
