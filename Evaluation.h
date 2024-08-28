#pragma once
#ifndef EVALUATION_H
#define EVALUATION_H

const int INF = std::numeric_limits<int>::max();
const int MATE_VALUE = 100000;


int get_piece_value_rigid(int piece)
{
    //pawn // knight // bishop // rook // queen // king
    int values[6] = { 100, 290, 300, 500, 900, 100000 };

    return values[piece];
}


int evaluate_position(Board* board)
{
    int result = 0;
    int who_to_move = -((board->turn * 2) - 1);

    uint64_t all_pieces = board->combine_black() | board->combine_white();

    unsigned long index;
    _BitScanReverse64(&index, board->white_pieces[5]);
    int white_king = static_cast<int>(index);

    _BitScanReverse64(&index, board->black_pieces[5]);
    int black_king = static_cast<int>(index);

    for (int i = 0; i < 6; i++)
    {
        uint64_t white_bitboard = board->white_pieces[i];
        uint64_t black_bitboard = board->black_pieces[i];

        int white_sq;
        while (white_bitboard) {
            white_sq = _tzcnt_u64(white_bitboard);
            result += get_piece_value_rigid(i);

            white_bitboard &= white_bitboard - 1;
        }

        int black_sq;
        while (black_bitboard) {
            black_sq = _tzcnt_u64(black_bitboard);
            result -= get_piece_value_rigid(i);

            black_bitboard &= black_bitboard - 1;
        }
    }

    return result * who_to_move;
}


#endif