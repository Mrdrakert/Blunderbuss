#ifndef DUMBHASH_H
#define DUMBHASH_H

#include <cstdint>
#include <iostream>


struct DumbHash
{
    uint64_t white_pieces[6];
    uint64_t black_pieces[6];

    uint64_t en_passant = 0;
    bool castling[4] = { 1, 1, 1, 1 };

    bool operator==(const DumbHash& other) const;

    DumbHash(const uint64_t(&wht_pieces)[6], const uint64_t(&blk_pieces)[6], uint64_t enpass, const bool(&cstling)[4]);
};

#endif  // DUMBHASH_H
