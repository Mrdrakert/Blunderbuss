#include "DumbHash.h"


bool DumbHash::operator==(const DumbHash& other) const
{
    for (int i = 0; i < 6; ++i)
        if (white_pieces[i] != other.white_pieces[i])
            return false;

    for (int i = 0; i < 6; ++i)
        if (black_pieces[i] != other.black_pieces[i])
            return false;

    if (en_passant != other.en_passant)
        return false;

    for (int i = 0; i < 4; ++i)
        if (castling[i] != other.castling[i])
            return false;

    return true;
}

DumbHash::DumbHash(const uint64_t(&wht_pieces)[6], const uint64_t(&blk_pieces)[6], uint64_t enpass, const bool(&cstling)[4])
{
    std::copy(std::begin(wht_pieces), std::end(wht_pieces), std::begin(white_pieces));
    std::copy(std::begin(blk_pieces), std::end(blk_pieces), std::begin(black_pieces));

    std::copy(std::begin(cstling), std::end(cstling), std::begin(castling));

    en_passant = enpass;
}