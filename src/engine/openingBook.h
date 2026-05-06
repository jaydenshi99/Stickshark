#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "../chess/board/board.h"

class OpeningBook {
public:
    explicit OpeningBook(const std::string& path);

    // Returns {src, dst, promoFlag} in engine coordinates, or nullopt if not in book.
    std::optional<std::tuple<int, int, int>> probe(const Board& board);

    bool isLoaded() const { return loaded; }

private:
    struct Entry {
        uint64_t key;
        uint16_t move;
        uint16_t weight;
    };

    std::vector<Entry> entries;
    bool loaded = false;

    static uint64_t computePolyglotHash(const Board& board);
    static std::tuple<int, int, int> decodeMove(uint16_t polyMove);
};
