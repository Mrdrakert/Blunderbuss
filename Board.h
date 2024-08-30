#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <random>
#include <sstream>
#include <iostream>
#include <fstream>
#include <intrin.h>

#include "Move.h"
#include "PieceAndBoard.h"
#include "AntiMove.h"
#include "DumbHash.h"

const int MAX_DEPTH = 32;


class Board
{
public:
    bool turn = 0;
    uint64_t white_pieces[6];
    uint64_t black_pieces[6];

    Board();
    void clear();
    void setup_board_from_fen(const std::string& fen);
    void make_move(Move move, bool reversible = 1);
    void make_move_opponent(Move move);
    void unmake_move();
    PieceAndBoard get_piece_bitboard(int square) const;
    uint64_t count_legal_moves_at_depth(int depth);
    uint64_t combine_white();
    uint64_t combine_black();
    int get_knight_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color);
    int get_king_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color);
    int get_pawn_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color);
    int get_bishop_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color, bool is_queen = 0);
    int get_rook_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color, bool is_queen = 0);
    int get_queen_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color);
    void print_chessboard();
    void set_piece(int square, int type);
    void create_moves(std::vector<Move>& or_moves, uint64_t dests_board, int from, uint64_t capture_mask, bool color, int piece_type);
    std::vector<Move> get_legal_moves(bool check_legality = 0, bool natural = 0, bool color = 0);
    bool if_check(bool color, int square = -1);
    void sort_moves(std::vector<Move>& moves, Move best_move, Move killer_move_1, Move killer_move_2);
    //int quiescence_search(int alpha, int beta, std::chrono::steady_clock::time_point end_time);
    int quiescence_search(int ply_from_root, int alpha, int beta);
    int search(int ply_from_root, int depth_left, int alpha, int beta);
    Move find_best_move_for_depth(int depth);
    Move find_best_move_with_time_limit(int time_limit_ms);

    void store_killer_move(int ply_from_root, Move move);
    
private:
    uint64_t positions_evaluated = 0;

    uint64_t en_passant = 0;
    bool castling[4] = { 1, 1, 1, 1 }; //white king, white queen, black king, black queen

    std::vector<AntiMove> anti_moves;
    std::vector<DumbHash> repetition_table;

    bool killer_moves_check[MAX_DEPTH][2];
    Move killer_moves[MAX_DEPTH][2];

    bool now_searching_for;

    bool timed_search = false;
    bool time_limit_reached = false;
    std::chrono::steady_clock::time_point end_time;
};


int store_correct_mate_score(int value, int depth);
int retrieve_correct_mate_score(int value, int depth);
bool is_mate_score(int value);
bool exists_more_than_once(const std::vector<DumbHash>& vec, DumbHash value);


#endif  // BOARD_H