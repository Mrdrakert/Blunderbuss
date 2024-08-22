#ifndef BLUNDERBUSS_H
#define BLUNDERBUSS_H

#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>

class Board;
struct Move;
enum class NodeType;
struct AntiMove;
struct PieceAndBoard;
struct TranspositionTableEntry;

class Board 
{
public:
    Board();
    void setup_board_from_fen(const std::string& fen);
    void make_move(Move move);
    void make_move_opponent(Move move);
    void unmake_move();
    PieceAndBoard get_piece_bitboard(int square) const;
    Move find_best_move_with_time_limit(int time_limit_ms);
    uint64_t combine_white();
    uint64_t combine_black();
    void get_knight_moves(std::vector<Move>& or_moves, int square, bool color);
    void get_king_moves(std::vector<Move>& or_moves, int square, bool color);
    void get_pawn_moves(std::vector<Move>& or_moves, int square, bool color);
    void get_bishop_moves(std::vector<Move>& or_moves, int square, bool color, bool is_queen = 0);
    void get_rook_moves(std::vector<Move>& or_moves, int square, bool color, bool is_queen = 0);
    void get_queen_moves(std::vector<Move>& or_moves, int square, bool color);
    void print_chessboard();
    void clear();
    void set_piece(int square, int type);
    void create_moves(std::vector<Move>& or_moves, uint64_t dests_board, int from, uint64_t capture_mask, bool color, int piece_type);
    std::vector<Move> get_legal_moves(bool natural = 0, bool color = 0);
    bool if_check(bool color, int square = -1);
    uint64_t compute_zobrist_hash();
    void store_transposition_table_entry(uint64_t zobrist_key, int ply, int depth, int score, NodeType type, Move best_move);
    bool probe_transposition_table(uint64_t zobrist_key, int depth, int ply, int alpha, int beta, int& value, Move& best_move);
    void clear_transposition_table();
    void print_checks();
    int evaluate_position(bool color);
    int evaluate_end(bool color, int depth);
    int quiescence_search(int alpha, int beta, std::chrono::steady_clock::time_point end_time);
    int negamax(int ply, int depth, int alpha, int beta, std::chrono::steady_clock::time_point end_time);
    uint64_t count_legal_moves_at_depth(int depth);
    void clear_anti_moves();

private:
    uint64_t white_pieces[6];
    uint64_t black_pieces[6];

    uint64_t en_passant = 0;
    bool castling[4] = { 1, 1, 1, 1 }; //white king, white queen, black king, black queen

    bool turn = 0;

    std::vector<AntiMove> anti_moves;

    std::unordered_map<uint64_t, TranspositionTableEntry> transposition_table;
};

struct Move 
{
    int from;
    int to;
    int piece_type;
    int type; //0 - normal, 1 - capture, 2 - castling, 3 - enpassant, 4 - pawn queening, 5 - pawn double move, 6 - pawn knighting, 7 - pawn bishoping, 8 - pawn rooking
    int capture_value;

    Move();
    Move(int the_from, int the_to, int pc_type, int the_type = 0, int cap_val = 0);

    bool operator==(const Move& other) const;
};

struct AntiMove
{
    uint64_t white_pieces[6];
    uint64_t black_pieces[6];

    uint64_t en_passant = 0;
    bool castling[4];

    bool turn = 0;

    AntiMove(const uint64_t wht_pieces[6], const uint64_t blk_pieces[6], uint64_t enpass, const bool cstling[4], bool trn);
};

struct PieceAndBoard
{
    int type;
    uint64_t board;

    PieceAndBoard(int the_type, uint64_t the_board);
};

enum class NodeType {
    EXACT,
    LOWERBOUND,
    UPPERBOUND
};

struct TranspositionTableEntry {
    int depth;
    int value;
    NodeType type;
    Move best_move;

    TranspositionTableEntry();
};


void load_data(uint64_t* values, std::string file_name);
void load_bitboards();
void initialize_zobrist_table();

const int MAX_DEPTH = 20;

#endif  // BLUNDERBUSS_H
