#include "DumbHash.h"
#include "ZobristKeys.h"


inline int bitscan_forward(uint64_t bb) {
    unsigned long index;
    _BitScanForward64(&index, bb);
    return static_cast<int>(index);
}

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

DumbHash::DumbHash(const uint64_t(&wht_pieces)[6], const uint64_t(&blk_pieces)[6], uint64_t enpass, const bool(&cstling)[4], bool turn)
{
    std::copy(std::begin(wht_pieces), std::end(wht_pieces), std::begin(white_pieces));
    std::copy(std::begin(blk_pieces), std::end(blk_pieces), std::begin(black_pieces));

    std::copy(std::begin(cstling), std::end(cstling), std::begin(castling));

    this->turn = turn;

    en_passant = enpass;
}

uint64_t DumbHash::hash()
{
    uint64_t hash_value = 0;

    for (int piece_type = 0; piece_type < 6; ++piece_type) 
    {
        uint64_t pieces = white_pieces[piece_type];
        while (pieces) 
        {
            int square = bitscan_forward(pieces);
            hash_value ^= piece_keys[0][piece_type][square];
            pieces &= pieces - 1;
        }
    }

    for (int piece_type = 0; piece_type < 6; ++piece_type)
    {
        uint64_t pieces = black_pieces[piece_type];
        while (pieces) 
        {
            int square = bitscan_forward(pieces);
            hash_value ^= piece_keys[1][piece_type][square];
            pieces &= pieces - 1;
        }
    }

    if (en_passant != 0) 
    {
        int square = bitscan_forward(en_passant);
        int file = square % 8;
        hash_value ^= en_passant_keys[file];
    }

    for (int i = 0; i < 4; ++i) 
    {
        if (castling[i]) 
        {
            hash_value ^= castling_keys[i];
        }
    }

    if (turn == 1) 
    {
        hash_value ^= side_to_move_key;
    }

    return hash_value;
}
