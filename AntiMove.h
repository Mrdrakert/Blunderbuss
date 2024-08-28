#ifndef ANTIMOVE_H
#define ANTIMOVE_H

#include <cstdint>


struct AntiMove
{
    uint64_t white_pieces[6];
    uint64_t black_pieces[6];

    uint64_t en_passant = 0;
    bool castling[4];

    bool turn = 0;

    AntiMove(const uint64_t(&wht_pieces)[6], const uint64_t(&blk_pieces)[6], uint64_t enpass, const bool(&cstling)[4], bool trn);
};

#endif  // ANTIMOVE_H