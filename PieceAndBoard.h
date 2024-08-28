#ifndef PIECEANDBOARD_H
#define PIECEANDBOARD_H

#include <cstdint>


struct PieceAndBoard
{
    int type;
    uint64_t board;

    PieceAndBoard(int the_type, uint64_t the_board);
};

#endif  // PIECEANDBOARD_H