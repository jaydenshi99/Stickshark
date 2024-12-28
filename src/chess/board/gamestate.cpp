#include "gamestate.h"

using namespace std;

Gamestate::Gamestate(int cptPiece) : attackBitboards {0ULL} {
    capturedPiece = cptPiece;
}
