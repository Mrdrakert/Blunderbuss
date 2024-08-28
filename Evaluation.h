#pragma once
#ifndef EVALUATION_H
#define EVALUATION_H

#include "PieceSquareTables.h"

const int INF = std::numeric_limits<int>::max();
const int MATE_VALUE = 100000;
const int DRAW_VALUE = -50;


int get_piece_value_rigid(int piece)
{
    //pawn // knight // bishop // rook // queen // king
    int values[6] = { 100, 290, 300, 500, 900, 100000 };

    return values[piece];
}


int get_square_for_table(int square, bool color)
{
    if (color == 1)
    {
        return square;
    }
    else
    {
        int row = square / 8;
        int col = square % 8;
        row = 7 - row;
        return row * 8 + col;
    }
}

int get_piece_placement_value(int square, int piece_type, bool color, int queens_strength, int pieces_strength)
{
    int value = 0;
    switch (piece_type)
    {
        case 0:
            value = pawn_table[get_square_for_table(square, color)];
            break;
        case 1:
            value = knight_table[get_square_for_table(square, color)];
            break;
        case 2:
            value = bishop_table[get_square_for_table(square, color)];
            break;
        case 3:
            value = rook_table[get_square_for_table(square, color)];
            break;
        case 4:
            value = queen_table[get_square_for_table(square, color)];
            break;
        case 5:
            if ((queens_strength >= 900 && pieces_strength < 610) || (queens_strength == 0))
                value = king_table_eg[get_square_for_table(square, color)];
            else
                value = king_table[get_square_for_table(square, color)];
            break;
    }
    
    return value;
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

    int queens = 0;
    int pieces_strength = 0;

    queens += get_piece_value_rigid(4) * __popcnt64(board->white_pieces[4]);
    queens += get_piece_value_rigid(4) * __popcnt64(board->black_pieces[4]);

    for (int i = 1; i < 4; i++)
    {
        pieces_strength += get_piece_value_rigid(i) * __popcnt64(board->white_pieces[i]);
        pieces_strength += get_piece_value_rigid(i) * __popcnt64(board->black_pieces[i]);
    }

    for (int i = 0; i < 6; i++)
    {
        uint64_t white_bitboard = board->white_pieces[i];
        uint64_t black_bitboard = board->black_pieces[i];

        int white_sq;
        while (white_bitboard) {
            white_sq = _tzcnt_u64(white_bitboard);
            result += get_piece_value_rigid(i);
            result += get_piece_placement_value(white_sq, i, 0, queens, pieces_strength);

            white_bitboard &= white_bitboard - 1;
        }

        int black_sq;
        while (black_bitboard) {
            black_sq = _tzcnt_u64(black_bitboard);
            result -= get_piece_value_rigid(i);
            result -= get_piece_placement_value(black_sq, i, 1, queens, pieces_strength);

            black_bitboard &= black_bitboard - 1;
        }
    }

    return result * who_to_move;
}


#endif