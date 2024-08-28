#include "AntiMove.h"

#include <cstdint>
#include <iostream>


AntiMove::AntiMove(const uint64_t(&wht_pieces)[6], const uint64_t(&blk_pieces)[6], uint64_t enpass, const bool(&cstling)[4], bool trn)
{
    std::copy(std::begin(wht_pieces), std::end(wht_pieces), std::begin(white_pieces));
    std::copy(std::begin(blk_pieces), std::end(blk_pieces), std::begin(black_pieces));
    std::copy(std::begin(cstling), std::end(cstling), std::begin(castling));

    turn = trn;
    en_passant = enpass;
}