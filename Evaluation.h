#pragma once
#ifndef EVALUATION_H
#define EVALUATION_H

#include "PieceSquareTables.h"

const int INF = std::numeric_limits<int>::max();
const int MATE_VALUE = 100000;
const int DRAW_VALUE = -50;

// when its = 1 then the engine tries to lose
const bool blunder_mode = 0;


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

int get_squares_distance(int square1, int square2)
{
    int row1 = square1 / 8;
    int col1 = square1 % 8;
    int row2 = square2 / 8;
    int col2 = square2 % 8;

    int rowDiff = std::abs(row1 - row2);
    int colDiff = std::abs(col1 - col2);

    return rowDiff + colDiff;
}

int count_doubled_pawns(uint64_t pawns_bitboard) 
{
    int doubled_pawns = 0;

    for (int file = 0; file < 8; ++file) 
    {
        uint64_t file_mask = 0x0101010101010101ULL << file;

        uint64_t pawns_in_file = pawns_bitboard & file_mask;

        if (pawns_in_file) 
        {
            int count = 0;
            while (pawns_in_file)
            {
                unsigned long index;
                _BitScanForward64(&index, pawns_in_file);
                pawns_in_file &= ~(1ULL << index);
                count++;
            }
            if (count > 1) 
            {
                doubled_pawns += count - 1;
            }
        }
    }

    return doubled_pawns;
}

int get_piece_placement_value(int square, int piece_type, bool color, int my_strength, int opp_strength)
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
            if (my_strength + opp_strength < 2000)
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

    int white_pieces_strength = 0;
    int black_pieces_strength = 0;

    for (int i = 1; i < 5; i++)
    {
        white_pieces_strength += get_piece_value_rigid(i) * __popcnt64(board->white_pieces[i]);
        black_pieces_strength += get_piece_value_rigid(i) * __popcnt64(board->black_pieces[i]);
    }

    int kings_distance = get_squares_distance(white_king, black_king);
    if (white_pieces_strength > 400 && black_pieces_strength < 300)
    {
        result += (15 - kings_distance) * 20;
    }
    else if (black_pieces_strength > 400 && white_pieces_strength < 300)
    {
        result -= (15 - kings_distance) * 20;
    }

    result -= board->DOUBLED_PAWN_PENALTY * count_doubled_pawns(board->white_pieces[0]);
    result += board->DOUBLED_PAWN_PENALTY * count_doubled_pawns(board->black_pieces[0]);


    for (int i = 0; i < 6; i++)
    {
        uint64_t white_bitboard = board->white_pieces[i];
        uint64_t black_bitboard = board->black_pieces[i];

        int white_sq;
        while (white_bitboard) {
            white_sq = _tzcnt_u64(white_bitboard);
            result += get_piece_value_rigid(i);
            result += get_piece_placement_value(white_sq, i, 0, white_pieces_strength, black_pieces_strength);

            white_bitboard &= white_bitboard - 1;
        }

        int black_sq;
        while (black_bitboard) {
            black_sq = _tzcnt_u64(black_bitboard);
            result -= get_piece_value_rigid(i);
            result -= get_piece_placement_value(black_sq, i, 1, black_pieces_strength, white_pieces_strength);

            black_bitboard &= black_bitboard - 1;
        }
    }

    if (blunder_mode)
        who_to_move = who_to_move * -1;

    return result * who_to_move;
}


#endif