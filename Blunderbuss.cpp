#include "Blunderbuss.h"
#include "SquareTables.h"

const int BOARD_SIZE = 64;
const int PIECE_TYPES = 12;
const int CASTLING_RIGHTS = 4;
const int EN_PASSANT = 64;
const int SIDE_TO_MOVE = 1;

uint64_t zobrist_table[BOARD_SIZE][PIECE_TYPES];
uint64_t zobrist_castling[CASTLING_RIGHTS];
uint64_t zobrist_en_passant[EN_PASSANT];
uint64_t zobrist_side_is_black;

bool time_limit_reached = false;

uint64_t positions_evaluated;
uint64_t positions_generated;
const int INF = std::numeric_limits<int>::max();
const int MATE_VALUE = 1000000;
const int NUM_LINES = 64;
uint64_t rook_moves[NUM_LINES];
uint64_t bishop_moves[NUM_LINES];
uint64_t queen_moves[NUM_LINES];
uint64_t knight_moves[NUM_LINES];
uint64_t king_moves[NUM_LINES];

uint64_t rook_moves_left[NUM_LINES];
uint64_t rook_moves_right[NUM_LINES];
uint64_t rook_moves_up[NUM_LINES];
uint64_t rook_moves_down[NUM_LINES];

uint64_t bishop_moves_left[NUM_LINES];
uint64_t bishop_moves_right[NUM_LINES];
uint64_t bishop_moves_up[NUM_LINES];
uint64_t bishop_moves_down[NUM_LINES];

uint64_t pawn_white_moves[NUM_LINES];
uint64_t pawn_black_moves[NUM_LINES];
uint64_t pawn_white_capture_moves[NUM_LINES];
uint64_t pawn_black_capture_moves[NUM_LINES];


Move::Move(int the_from, int the_to, int pc_type, int the_type, int cap_val)
    : from(the_from), to(the_to), piece_type(pc_type), type(the_type), capture_value(cap_val) {}

Move::Move()
{
    from = -1;
    to = -1;
    piece_type = -1;
    type = -1;
    capture_value = -1;
}

bool Move::operator==(const Move& other) const
{
    return from == other.from && to == other.to && piece_type == other.piece_type && type == other.piece_type && capture_value == other.capture_value; // Add other necessary comparisons
}

AntiMove::AntiMove(const uint64_t wht_pieces[6], const uint64_t blk_pieces[6], uint64_t enpass, const bool cstling[4], bool trn) {
    for (int i = 0; i < 6; ++i) {
        white_pieces[i] = wht_pieces[i];
        black_pieces[i] = blk_pieces[i];
    }
    for (int i = 0; i < 4; ++i) {
        castling[i] = cstling[i];
    }

    turn = trn;
    en_passant = enpass;
}

PieceAndBoard::PieceAndBoard(int the_type, uint64_t the_board)
    : type(the_type), board(the_board) {}



TranspositionTableEntry::TranspositionTableEntry()
{
    depth = -1;
    value = -1;
    type = NodeType::EXACT;
    best_move = Move();
}





Board::Board()
    :white_pieces{ 0x00FF000000000000, 0x4200000000000000, 0x2400000000000000,
                    0x8100000000000000, 0x1000000000000000, 0x0800000000000000 },
    black_pieces{ 0x000000000000FF00, 0x0000000000000042, 0x0000000000000024,
                    0x0000000000000081, 0x0000000000000010, 0x0000000000000008 },
    en_passant(0x0000000000000000)
{
    this->anti_moves.reserve(10);
    load_bitboards();
    initialize_zobrist_table();
}


PieceAndBoard Board::get_piece_bitboard(int square) const
{
    uint64_t bit = 1ULL << square;
    for (int i = 0; i < 6; i++)
    {
        if (white_pieces[i] & bit) return PieceAndBoard(i, bit);
        if (black_pieces[i] & bit) return PieceAndBoard(6 + i, bit);
    }
    return PieceAndBoard(-1, bit);
}

uint64_t Board::combine_white()
{
    return white_pieces[0] | white_pieces[1] | white_pieces[2] | white_pieces[3] | white_pieces[4] | white_pieces[5];
}

uint64_t Board::combine_black()
{
    return black_pieces[0] | black_pieces[1] | black_pieces[2] | black_pieces[3] | black_pieces[4] | black_pieces[5];
}
    
void Board::get_knight_moves(std::vector<Move>& or_moves, int square, bool color) //0 = white, 1 = black
{
    uint64_t result;
    uint64_t captures;
    uint64_t all_white = this->combine_white();
    uint64_t all_black = this->combine_black();

    if (color == 0)
    {
        result = (knight_moves[square] | all_white) - all_white;
        captures = result & all_black;
    }
    else
    {
        result = (knight_moves[square] | all_black) - all_black;
        captures = result & all_white;
    }

    create_moves(or_moves, result, square, captures, color, 1);
}

void Board::get_king_moves(std::vector<Move>& or_moves, int square, bool color) //0 = white, 1 = black
{
    uint64_t result;
    uint64_t captures;
    uint64_t all_white = this->combine_white();
    uint64_t all_black = this->combine_black();

    if (color == 0)
    {
        result = (king_moves[square] | all_white) - all_white;
        captures = result & all_black;
    }
    else
    {
        result = (king_moves[square] | all_black) - all_black;
        captures = result & all_white;
    }

    create_moves(or_moves, result, square, captures, color, 5);
}

void Board::get_pawn_moves(std::vector<Move>& or_moves, int square, bool color) //0 = white, 1 = black
{
    uint64_t all_white = this->combine_white();
    uint64_t all_black = this->combine_black();
    uint64_t all_pieces = all_white | all_black;
    uint64_t captures;
    uint64_t forwards;
    uint64_t block_mask;

    if (color == 0)
    {
        captures = pawn_white_capture_moves[square] & (all_black | this->en_passant);
        forwards = (pawn_white_moves[square] | all_pieces) - all_pieces;

        if (square / 8 == 1)
        {
            block_mask = (0x0000000000FF0000 & all_pieces) << 8;
            forwards = (forwards | block_mask) - block_mask;
        }
            
    }
    else
    {
        captures = pawn_black_capture_moves[square] & (all_white | this->en_passant);
        forwards = (pawn_black_moves[square] | all_pieces) - all_pieces;

        if (square / 8 == 6)
        {
            block_mask = (0x0000FF0000000000 & all_pieces) >> 8;
            forwards = (forwards | block_mask) - block_mask;
        }
    }

    create_moves(or_moves, captures | forwards, square, all_pieces, color, 0);
}

void Board::get_rook_moves(std::vector<Move>& or_moves, int square, bool color, bool is_queen)
{
    uint64_t all_white = this->combine_white();
    uint64_t all_black = this->combine_black();
    uint64_t all_pieces = all_white | all_black;

    uint64_t rook_up = rook_moves_up[square];
    uint64_t rook_down = rook_moves_down[square];
    uint64_t rook_left = rook_moves_left[square];
    uint64_t rook_right = rook_moves_right[square];

    uint64_t blocker_up = (all_pieces & rook_up);
    int blocker_index = findLeastSignificantBitIndex(blocker_up);
    uint64_t rook_attacks_up = (blocker_up == 0) ? rook_up : (rook_up & ~rook_moves_up[blocker_index]);

    uint64_t blocker_down = (all_pieces & rook_down);
    blocker_index = findMostSignificantBitIndex(blocker_down);
    uint64_t rook_attacks_down = (blocker_down == 0) ? rook_down : (rook_down & ~rook_moves_down[blocker_index]);

    uint64_t blocker_left = (all_pieces & rook_left);
    blocker_index = findMostSignificantBitIndex(blocker_left);
    uint64_t rook_attacks_left = (blocker_left == 0) ? rook_left : (rook_left & ~rook_moves_left[blocker_index]);

    uint64_t blocker_right = (all_pieces & rook_right);
    blocker_index = findLeastSignificantBitIndex(blocker_right);
    uint64_t rook_attacks_right = (blocker_right == 0) ? rook_right : (rook_right & ~rook_moves_right[blocker_index]);

    uint64_t rook_attacks = rook_attacks_up | rook_attacks_down | rook_attacks_left | rook_attacks_right;
    rook_attacks = color == 0 ? (rook_attacks & ~all_white) : (rook_attacks & ~all_black);
    uint64_t captures = color == 0 ? (rook_attacks & all_black) : (rook_attacks & all_white);

    create_moves(or_moves, rook_attacks, square, captures, color, 3+is_queen);
}

void Board::get_bishop_moves(std::vector<Move>& or_moves, int square, bool color, bool is_queen)
{
    uint64_t all_white = this->combine_white();
    uint64_t all_black = this->combine_black();
    uint64_t all_pieces = all_white | all_black;

    uint64_t bishop_up = bishop_moves_up[square];
    uint64_t bishop_down = bishop_moves_down[square];
    uint64_t bishop_left = bishop_moves_left[square];
    uint64_t bishop_right = bishop_moves_right[square];

    uint64_t blocker_up = (all_pieces & bishop_up);
    int blocker_index = findLeastSignificantBitIndex(blocker_up);
    uint64_t bishop_attacks_up = (blocker_up == 0) ? bishop_up : (bishop_up & ~bishop_moves_up[blocker_index]);

    uint64_t blocker_down = (all_pieces & bishop_down);
    blocker_index = findMostSignificantBitIndex(blocker_down);
    uint64_t bishop_attacks_down = (blocker_down == 0) ? bishop_down : (bishop_down & ~bishop_moves_down[blocker_index]);

    uint64_t blocker_left = (all_pieces & bishop_left);
    blocker_index = findMostSignificantBitIndex(blocker_left);
    uint64_t bishop_attacks_left = (blocker_left == 0) ? bishop_left : (bishop_left & ~bishop_moves_left[blocker_index]);

    uint64_t blocker_right = (all_pieces & bishop_right);
    blocker_index = findLeastSignificantBitIndex(blocker_right);
    uint64_t bishop_attacks_right = (blocker_right == 0) ? bishop_right : (bishop_right & ~bishop_moves_right[blocker_index]);

    uint64_t bishop_attacks = bishop_attacks_up | bishop_attacks_down | bishop_attacks_left | bishop_attacks_right;
    bishop_attacks = color == 0 ? (bishop_attacks & ~all_white) : (bishop_attacks & ~all_black);
    uint64_t captures = color == 0 ? (bishop_attacks & all_black) : (bishop_attacks & all_white);

    create_moves(or_moves, bishop_attacks, square, captures, color, 2+2*is_queen);
}

void Board::get_queen_moves(std::vector<Move>& or_moves, int square, bool color)
{
    this->get_rook_moves(or_moves, square, color, 1);
    this->get_bishop_moves(or_moves, square, color, 1);
}

void Board::print_chessboard()
{
    const char* piece_symbols[12] = { "P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k"};

    for (int rank = 7; rank >= 0; --rank) 
    {
        for (int file = 0; file < 8; ++file) 
        {
            int square = rank * 8 + file;
            PieceAndBoard piece = this->get_piece_bitboard(square);

            if (piece.type == -1) 
            {
                std::cout << ". ";  // Empty square
            }
            else 
            {
                std::cout << piece_symbols[piece.type] << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Board::clear()
{
    time_limit_reached = false;
    for (int i = 0; i < 6; i++)
    {
        this->white_pieces[i] = 0x0000000000000000;
        this->black_pieces[i] = 0x0000000000000000;
    }
    this->en_passant = 0;
    for (int i = 0; i < 4; i++)
    {
        this->castling[i] = 0;
    }
}

void Board::setup_board_from_fen(const std::string& fen)
{
    this->clear_anti_moves();
    this->clear_transposition_table();
    this->clear();

    std::istringstream fen_stream(fen);
    std::string board_part, active_color, castling, en_passant;

    fen_stream >> board_part >> active_color >> castling >> en_passant;

    int rank = 7;
    int file = 0;

    for (char c : board_part) 
    {
        if (c == '/') 
        {
            rank--;
            file = 0;
        }
        else if (isdigit(c)) 
        {
            file += c - '0';
        }
        else
        {
            int square = rank * 8 + file;
            switch (c) 
            {
                case 'P': this->set_piece(square, 0); break;
                case 'N': this->set_piece(square, 1); break;
                case 'B': this->set_piece(square, 2); break;
                case 'R': this->set_piece(square, 3); break;
                case 'Q': this->set_piece(square, 4); break;
                case 'K': this->set_piece(square, 5); break;
                case 'p': this->set_piece(square, 6); break;
                case 'n': this->set_piece(square, 7); break;
                case 'b': this->set_piece(square, 8); break;
                case 'r': this->set_piece(square, 9); break;
                case 'q': this->set_piece(square, 10); break;
                case 'k': this->set_piece(square, 11); break;
            }
            file++;
        }
    }

    if (active_color == "w") 
    {
        this->turn = 0;
    }
    else if (active_color == "b") 
    {
        this->turn = 1;
    }

    for (char c : castling) 
    {
        switch (c) 
        {
            case 'K': this->castling[0] = 1; break;
            case 'Q': this->castling[1] = 1; break;
            case 'k': this->castling[2] = 1; break;
            case 'q': this->castling[3] = 1; break;
        }
    }

    if (en_passant != "-") 
    {
        int file = en_passant[0] - 'a';
        int rank = en_passant[1] - '1';
        int square = rank * 8 + file;
        this->en_passant = (1ULL << square);;
    }
    else 
    {
        this->en_passant = 0;
    }
}

void Board::set_piece(int square, int type)
{
    if (type < 6)
    {
        this->white_pieces[type] |= (1ULL << square);
    }
    else
    {
        this->black_pieces[type - 6] |= (1ULL << square);
    }
}

void Board::create_moves(std::vector<Move>& or_moves, uint64_t dests_board, int from, uint64_t capture_mask, bool color, int piece_type)
{
    std::array<int, 64> dests;
    std::array<int, 64> captures;
    int count = 0;

    unsigned long en_passant_index;
    _BitScanForward64(&en_passant_index, this->en_passant);

    unsigned long index;

    while (dests_board != 0)
    {
        if (_BitScanForward64(&index, dests_board))
        {
            dests[count] = static_cast<int>(index);
            captures[count] = (capture_mask >> index) & 1;
            dests_board &= (dests_board - 1);
            count++;
        }
    }
        
    int color_bonus = color == 0 ? 0 : 6;
    bool did_black_castles = 0;
    bool did_white_castles = 0;

        

    for (int i = 0; i < count; i++)
    {
        PieceAndBoard captured_piece = get_piece_bitboard(dests[i]);
        int capture_value = 0;
        if (captured_piece.type != -1)
        {
            captured_piece.type = (captured_piece.type > 5) ? captured_piece.type - 6 : captured_piece.type;
            capture_value = 10 * get_piece_value(captured_piece.type) - get_piece_value(piece_type);
        }

        if (piece_type == 0)
        {
            if (abs(dests[i] - from) == 16)
            {
                or_moves.emplace_back(Move(from, dests[i], piece_type + color_bonus, 5, capture_value));
            }
            else if (dests[i] / 8 == 0 || dests[i] / 8 == 7)
            {
                or_moves.emplace_back(Move(from, dests[i], piece_type + color_bonus, 4, capture_value));
                or_moves.emplace_back(Move(from, dests[i], piece_type + color_bonus, 6, capture_value));
                or_moves.emplace_back(Move(from, dests[i], piece_type + color_bonus, 7, capture_value));
                or_moves.emplace_back(Move(from, dests[i], piece_type + color_bonus, 8, capture_value));
            }
            else if (en_passant_index == dests[i])
            {
                or_moves.emplace_back(Move(from, dests[i], piece_type + color_bonus, 3, 900));
            }
            else
            {
                or_moves.emplace_back(Move(from, dests[i], piece_type + color_bonus, captures[i], capture_value));
            }
        }
        else if (piece_type == 5)
        {
            if (color == 0 && from == 4 && did_white_castles == 0) // White king castling
            {  
                did_white_castles = 1;
                uint64_t check = ((this->combine_white() | this->combine_black()) & ((1ULL << 5) | (1ULL << 6)));
                if (this->castling[0] && !if_check(0) && !if_check(0, 5) && !if_check(0, 6) && !check) {
                    or_moves.emplace_back(Move(from, 6, piece_type + color_bonus, 2));  // White king-side castling
                }
                check = ((this->combine_white() | this->combine_black()) & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3)));
                if (this->castling[1] && !if_check(0) && !if_check(0, 3) && !if_check(0, 2) && !check) {
                    or_moves.emplace_back(Move(from, 2, piece_type + color_bonus, 2));  // White queen-side castling
                }
            }
            else if (color == 1 && from == 60 && did_black_castles == 0) // Black king castling
            { 
                did_black_castles = 1;
                uint64_t check = ((this->combine_white() | this->combine_black()) & ((1ULL << 61) | (1ULL << 62)));
                if (this->castling[2] && !if_check(1) && !if_check(1, 61) && !if_check(1, 62) && !check)
                {
                    or_moves.emplace_back(Move(from, 62, piece_type + color_bonus, 2));  // Black king-side castling
                }
                check = ((this->combine_white() | this->combine_black()) & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59)));
                if (this->castling[3] && !if_check(1) && !if_check(1, 59) && !if_check(1, 58) && !check) {
                    or_moves.emplace_back(Move(from, 58, piece_type + color_bonus, 2));  // Black queen-side castling
                }
            }
            or_moves.emplace_back(Move(from, dests[i], piece_type + color_bonus, captures[i], capture_value));
        }
        else
        {
            or_moves.emplace_back(Move(from, dests[i], piece_type + color_bonus, captures[i], capture_value));
        }
    }
}

std::vector<Move> Board::get_legal_moves(bool natural, bool color)
{
    if (natural == 0)
    {
        color = this->turn;
    }
    std::vector<Move> pseudo_legal;
    pseudo_legal.reserve(64);

    uint64_t pieces[6];
    if (color == 0) 
    {
        for (int i = 0; i < 6; ++i) {
            pieces[i] = white_pieces[i];
        }
    }
    else 
    {
        for (int i = 0; i < 6; ++i) {
            pieces[i] = black_pieces[i];
        }
    }

    for (int piece_type = 0; piece_type < 6; piece_type++) {
        uint64_t bitboard = pieces[piece_type];
        while (bitboard) {
            int square = findLeastSignificantBitIndex(bitboard);

            switch (piece_type) {
            case 0:
                get_pawn_moves(pseudo_legal, square, color);
                break;
            case 1:
                get_knight_moves(pseudo_legal, square, color);
                break;
            case 2:
                get_bishop_moves(pseudo_legal, square, color);
                break;
            case 3:
                get_rook_moves(pseudo_legal, square, color);
                break;
            case 4:
                get_queen_moves(pseudo_legal, square, color);
                break;
            case 5:
                get_king_moves(pseudo_legal, square, color);
                break;
            }
            bitboard &= bitboard - 1;
        }
    }

    int it = 0;
    while(it < pseudo_legal.size())
    {
        this->make_move(pseudo_legal[it]);
        if (this->if_check(!turn))
        {
            std::swap(pseudo_legal[it], pseudo_legal.back());
            pseudo_legal.pop_back();
            it--;
        }
        it++;
            
        this->unmake_move();
    }

    return pseudo_legal;
}

bool Board::if_check(bool color, int square)
{
    int king_square;
    if (square == -1)
    {
        if (color == 0)
            king_square = findLeastSignificantBitIndex(this->white_pieces[5]);
        else
            king_square = findLeastSignificantBitIndex(this->black_pieces[5]);
    }
    else
        king_square = square;

    uint64_t knight_captures;
    uint64_t king_captures;
    uint64_t pawn_captures;
    uint64_t rook_captures;
    uint64_t bishop_captures;
    uint64_t result;
    uint64_t all_white = this->combine_white();
    uint64_t all_black = this->combine_black();
    uint64_t all_pieces = all_white | all_black;

    if (color == 0)
    {
        result = (knight_moves[king_square] | all_white) - all_white;
        knight_captures = result & all_black;
        if (knight_captures & black_pieces[1])
            return true;

        result = (king_moves[king_square] | all_white) - all_white;
        king_captures = result & all_black;
        if ((king_captures & black_pieces[5]) || (king_captures & black_pieces[4]))
            return true;

        pawn_captures = pawn_white_capture_moves[king_square] & all_black;
        if (pawn_captures & black_pieces[0])
            return true;
    }
    else
    {
        result = (knight_moves[king_square] | all_black) - all_black;
        knight_captures = result & all_white;
        if (knight_captures & white_pieces[1])
            return true;

        result = (king_moves[king_square] | all_black) - all_black;
        king_captures = result & all_white;
        if ((king_captures & white_pieces[5]) || (king_captures & white_pieces[4]))
            return true;

        pawn_captures = pawn_black_capture_moves[king_square] & all_white;
        if (pawn_captures & white_pieces[0])
            return true;
    }

    uint64_t rook_up = rook_moves_up[king_square];
    uint64_t rook_down = rook_moves_down[king_square];
    uint64_t rook_left = rook_moves_left[king_square];
    uint64_t rook_right = rook_moves_right[king_square];

    uint64_t blocker_up = (all_pieces & rook_up);
    int blocker_index = findLeastSignificantBitIndex(blocker_up);
    uint64_t rook_attacks_up = (blocker_up == 0) ? rook_up : (rook_up & ~rook_moves_up[blocker_index]);

    uint64_t blocker_down = (all_pieces & rook_down);
    blocker_index = findMostSignificantBitIndex(blocker_down);
    uint64_t rook_attacks_down = (blocker_down == 0) ? rook_down : (rook_down & ~rook_moves_down[blocker_index]);

    uint64_t blocker_left = (all_pieces & rook_left);
    blocker_index = findMostSignificantBitIndex(blocker_left);
    uint64_t rook_attacks_left = (blocker_left == 0) ? rook_left : (rook_left & ~rook_moves_left[blocker_index]);

    uint64_t blocker_right = (all_pieces & rook_right);
    blocker_index = findLeastSignificantBitIndex(blocker_right);
    uint64_t rook_attacks_right = (blocker_right == 0) ? rook_right : (rook_right & ~rook_moves_right[blocker_index]);

    result = rook_attacks_up | rook_attacks_down | rook_attacks_left | rook_attacks_right;
    if (color == 0)
    {
        rook_captures = result & all_black;
        if ((rook_captures & black_pieces[3]) || (rook_captures & black_pieces[4]))
            return true;
    }
    else
    {
        rook_captures = result & all_white;
        if ((rook_captures & white_pieces[3]) || (rook_captures & white_pieces[4]))
            return true;
    }


    uint64_t bishop_up = bishop_moves_up[king_square];
    uint64_t bishop_down = bishop_moves_down[king_square];
    uint64_t bishop_left = bishop_moves_left[king_square];
    uint64_t bishop_right = bishop_moves_right[king_square];

    blocker_up = (all_pieces & bishop_up);
    blocker_index = findLeastSignificantBitIndex(blocker_up);
    uint64_t bishop_attacks_up = (blocker_up == 0) ? bishop_up : (bishop_up & ~bishop_moves_up[blocker_index]);

    blocker_down = (all_pieces & bishop_down);
    blocker_index = findMostSignificantBitIndex(blocker_down);
    uint64_t bishop_attacks_down = (blocker_down == 0) ? bishop_down : (bishop_down & ~bishop_moves_down[blocker_index]);

    blocker_left = (all_pieces & bishop_left);
    blocker_index = findMostSignificantBitIndex(blocker_left);
    uint64_t bishop_attacks_left = (blocker_left == 0) ? bishop_left : (bishop_left & ~bishop_moves_left[blocker_index]);

    blocker_right = (all_pieces & bishop_right);
    blocker_index = findLeastSignificantBitIndex(blocker_right);
    uint64_t bishop_attacks_right = (blocker_right == 0) ? bishop_right : (bishop_right & ~bishop_moves_right[blocker_index]);

    result = bishop_attacks_up | bishop_attacks_down | bishop_attacks_left | bishop_attacks_right;
    if (color == 0)
    {
        bishop_captures = result & all_black;
        if ((bishop_captures & black_pieces[2]) || (bishop_captures & black_pieces[4]))
            return true;
    }
    else
    {
        bishop_captures = result & all_white;
        if ((bishop_captures & white_pieces[2]) || (bishop_captures & white_pieces[4]))
            return true;
    }

    return false;
}

void Board::make_move_opponent(Move move)
{
    std::vector<Move> moves = this->get_legal_moves();

    for (int i = 0; i < moves.size(); i++)
    {
        if (moves[i].from == move.from && moves[i].to == move.to)
        {
            this->make_move(moves[i]);
            return;
        }
    }

}

void Board::make_move(Move move)
{
    this->anti_moves.emplace_back(AntiMove(this->white_pieces, this->black_pieces, this->en_passant, this->castling, this->turn));

    int from_square = move.from;
    int to_square = move.to;
    int piece_type = move.piece_type;
    bool color = piece_type < 6 ? 0 : 1;

    PieceAndBoard piece_before = get_piece_bitboard(from_square);
    PieceAndBoard captured_piece = get_piece_bitboard(to_square);
    int captured_from = to_square;
    int captured_type = captured_piece.type;
    bool cstl1 = this->castling[0];
    bool cstl2 = this->castling[1];
    bool cstl3 = this->castling[2];
    bool cstl4 = this->castling[3];
    uint64_t enpass = this->en_passant;
    this->en_passant = 0;

    uint64_t* piece_board = color == 0 ? &white_pieces[piece_type] : &black_pieces[piece_type - 6];

    if (move.type == 1 || move.type == 4 || move.type == 6 || move.type == 7 || move.type == 8) // Capture
    { 
        PieceAndBoard captured_piece = get_piece_bitboard(to_square);
        if (captured_piece.type != -1) 
        {
            uint64_t* captured_board = captured_piece.type < 6 ? &white_pieces[captured_piece.type] : &black_pieces[captured_piece.type - 6];
            *captured_board &= ~(1ULL << to_square); // Remove captured piece
        }
    }
    else if (move.type == 3) // En passant
    { 
        int captured_pawn_square = to_square + (color == 0 ? -8 : 8);
            
        captured_piece = get_piece_bitboard(captured_pawn_square);
        captured_type = captured_piece.type;

        captured_from = captured_pawn_square;
        black_pieces[0] &= ~(1ULL << captured_pawn_square); // Remove the captured pawn for black
        white_pieces[0] &= ~(1ULL << captured_pawn_square); // Remove the captured pawn for white
    }
    else if (move.type == 5)
    {
        int en_passant_square = (from_square + to_square) / 2;
        this->en_passant = (1ULL << en_passant_square);
    }

    *piece_board &= ~(1ULL << from_square);
    *piece_board |= (1ULL << to_square);

    if (move.type == 4 || move.type == 6 || move.type == 7 || move.type == 8)  // Pawn promotion
    {
        int pc_board = move.type == 4 ? 4 : move.type - 5;
        uint64_t* promotion_board = color == 0 ? &white_pieces[move.piece_type] : &black_pieces[move.piece_type - 6];
        *promotion_board &= ~(1ULL << to_square);
        if (color == 0) 
            white_pieces[pc_board] |= (1ULL << to_square); // Promote to queen for white
        else 
            black_pieces[pc_board] |= (1ULL << to_square); // Promote to queen for black
    }

    if (move.type == 2) { // Castling
        if (to_square == 6) { // White kingside castling
            white_pieces[3] &= ~(1ULL << 7);
            white_pieces[3] |= (1ULL << 5);
        }
        else if (to_square == 2) { // White queenside castling
            white_pieces[3] &= ~(1ULL << 0);
            white_pieces[3] |= (1ULL << 3);
        }
        else if (to_square == 62) { // Black kingside castling
            black_pieces[3] &= ~(1ULL << 63);
            black_pieces[3] |= (1ULL << 61);
        }
        else if (to_square == 58) { // Black queenside castling
            black_pieces[3] &= ~(1ULL << 56);
            black_pieces[3] |= (1ULL << 59);
        }
    }

    if (piece_type == 5 || piece_type == 11)  // King moved
    {
        if (color == 0) // White loses both castling rights
        {
            castling[0] = 0;
            castling[1] = 0;
        }
        else // Black loses both castling rights
        {
            castling[2] = 0;
            castling[3] = 0; 
        }
    }
    if (from_square == 0 || to_square == 0) castling[1] = 0; // White queenside rook moved/captured
    if (from_square == 7 || to_square == 7) castling[0] = 0; // White kingside rook moved/captured
    if (from_square == 56 || to_square == 56) castling[3] = 0; // Black queenside rook moved/captured
    if (from_square == 63 || to_square == 63) castling[2] = 0; // Black kingside rook moved/captured

    this->turn = !turn;
}

void Board::unmake_move()
{
    AntiMove move = this->anti_moves.back();
    this->anti_moves.pop_back();

    for (int i = 0; i < 6; ++i) {
        this->white_pieces[i] = move.white_pieces[i];
        this->black_pieces[i] = move.black_pieces[i];
    }
    for (int i = 0; i < 4; ++i) {
        this->castling[i] = move.castling[i];
    }
    this->turn = move.turn;
    this->en_passant = move.en_passant;
}

uint64_t Board::compute_zobrist_hash()
{
    uint64_t hash = 0;

    for (int square = 0; square < BOARD_SIZE; ++square)
    {
        PieceAndBoard piece_board = this->get_piece_bitboard(square);
        if (piece_board.type != -1)
        {
            hash ^= zobrist_table[square][piece_board.type];
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (this->castling[i])
        {
            hash ^= zobrist_castling[this->castling[i]];
        }
    }

    if (this->en_passant != -1)
    {
        unsigned long index;
        _BitScanForward64(&index, this->en_passant);
        hash ^= zobrist_en_passant[index];
    }

    if (this->turn == 1)
    {
        hash ^= zobrist_side_is_black;
    }

    return hash;
}

void Board::store_transposition_table_entry(uint64_t zobrist_key, int ply, int depth, int score, NodeType type, Move best_move) {
    TranspositionTableEntry entry;
    entry.depth = depth;
    entry.best_move = best_move;

    entry.type = type;

    entry.value = store_correct_mate_score(score, ply);
    this->transposition_table[zobrist_key] = entry;
}


bool Board::probe_transposition_table(uint64_t zobrist_key, int depth, int ply, int alpha, int beta, int& value, Move& best_move)
{
    if (this->transposition_table.find(zobrist_key) != this->transposition_table.end())
    {
        TranspositionTableEntry entry = transposition_table[zobrist_key];
        best_move = entry.best_move;
        int corrected = retrieve_correct_mate_score(entry.value, ply);
        if (entry.depth >= depth)
        {
            if (entry.type == NodeType::EXACT)
            {
                value = corrected;
                return true;
            }
            if (entry.type == NodeType::UPPERBOUND && entry.value <= alpha)
            {
                value = corrected;
                return true;
            }
            if (entry.type == NodeType::LOWERBOUND && entry.value >= beta)
            {
                value = corrected;
                return true;
            }


        }
    }
    return false;
}

void Board::clear_transposition_table() {
    this->transposition_table.clear();
}

void Board::clear_anti_moves() {
    this->anti_moves.clear();
}

void Board::print_checks()
{
    std::cout << "Is white in check: " << this->if_check(0) << std::endl;
    std::cout << "Is black in check: " << this->if_check(1) << std::endl;
}

int Board::evaluate_position(bool color)
{
    int who_to_move = -((this->turn * 2) - 1);
    uint64_t num = this->black_pieces[0];
    int black_values[6];
    int white_values[6];

    int result = 0;

    int piece_strengh_score = 0;
    for (int i = 1; i < 5; i++)
    {
        white_values[i] = __popcnt64(this->white_pieces[i]);
        black_values[i] = __popcnt64(this->black_pieces[i]);

        piece_strengh_score += white_values[i] * get_piece_value(i);
        piece_strengh_score += black_values[i] * get_piece_value(i);
    }

    for (int i = 0; i < 6; i++)
    {
        uint64_t white_bitboard = this->white_pieces[i];
        uint64_t black_bitboard = this->black_pieces[i];

        while (white_bitboard) {
            int sq = _tzcnt_u64(white_bitboard);
            result += get_piece_square_value(i, sq, piece_strengh_score, 0);
            result += get_piece_value(i);
            white_bitboard &= white_bitboard - 1;
        }

        while (black_bitboard) {
            int sq = _tzcnt_u64(black_bitboard);
            result -= get_piece_square_value(i, sq, piece_strengh_score, 1);
            result -= get_piece_value(i);;
            black_bitboard &= black_bitboard - 1;
        }
    }

    return result * who_to_move;
}

int Board::evaluate_end(bool color, int depth)
{
    if (this->if_check(color))
    {
        int checkmate_value = -MATE_VALUE + depth;
        return checkmate_value;
    }
    return 0;
}

int Board::quiescence_search(int alpha, int beta, std::chrono::steady_clock::time_point end_time)
{
    positions_evaluated += 1;
    int stand_pat = this->evaluate_position(this->turn);

    if (stand_pat >= beta) 
    {
        return beta;
    }
    if (stand_pat > alpha) 
    {
        alpha = stand_pat;
    }

    std::vector<Move> moves = this->get_legal_moves();

    moves.erase(std::remove_if(moves.begin(), moves.end(), [](const Move& m) 
            { return m.capture_value <= 0;  }), moves.end() );

    std::sort(moves.begin(), moves.end(), compare_moves);

    for (const Move& move : moves)
    {
        this->make_move(move);
        int eval = -quiescence_search(-beta, -alpha, end_time);
        this->unmake_move();

        if (eval >= beta)
        {
            return beta;
        }

        if (eval > alpha)
        {
            alpha = eval;
        }

        if (std::chrono::steady_clock::now() >= end_time) {
            time_limit_reached = true;
            break;
        }
    }

    return alpha;
}


int Board::negamax(int ply, int depth, int alpha, int beta, std::chrono::steady_clock::time_point end_time)
{
    if (depth == 0 || time_limit_reached)
    {
        positions_evaluated += 1;
        return quiescence_search(alpha, beta, end_time);
        //return this->evaluate_position(this->turn);
    }

    uint64_t zobrist_key = this->compute_zobrist_hash();
    int value;
    int originalAlpha = alpha;
    Move best_move;

    if (this->probe_transposition_table(zobrist_key, depth, ply, alpha, beta, value, best_move))
    {
        return value;
    }
            
    std::vector<Move> moves = this->get_legal_moves();

    if (depth == 1)
        positions_generated += moves.size();

    if (moves.empty())
    {
        positions_evaluated += 1;
        return this->evaluate_end(this->turn, ply);
    }

    std::sort(moves.begin(), moves.end(), compare_moves);
    prioritize_best_move(moves, best_move);

    NodeType evaluation_bound = NodeType::UPPERBOUND;

    for (const Move& move : moves) 
    {
        this->make_move(move);
        int eval = -negamax(ply + 1, depth - 1, -beta, -alpha, end_time);
        this->unmake_move();

        if (eval >= beta) 
        {
            this->store_transposition_table_entry(zobrist_key, depth, ply, beta, NodeType::LOWERBOUND, move);
            return beta;
        }

        if (eval > alpha)
        {
            evaluation_bound = NodeType::EXACT;
            best_move = move;
            alpha = eval;
        }


        if (std::chrono::steady_clock::now() >= end_time) {
            time_limit_reached = true;
            break;
        }
    }

    this->store_transposition_table_entry(zobrist_key, depth, ply, alpha, evaluation_bound, best_move);
    return alpha;
}

Move Board::find_best_move_with_time_limit(int time_limit_ms)
{
    int bestValue = -INF;
    Move best_move = Move(-1, -1, -1, -1);
    std::vector<Move> moves = this->get_legal_moves();

    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::milliseconds(time_limit_ms);

    int depths_searched = 0;

    for (int depth = 2; depth <= MAX_DEPTH; depth++) 
    {
        int current_best_value = -INF;
        Move current_best_move;

        for (const Move& move : moves) 
        {
            this->make_move(move);
            int eval = -negamax(1, depth - 1, -INF, INF, end_time);

            if (depth == 15)
            {
                //std::cout << "val: " << eval << " from: " << move.from << " to: " << move.to << std::endl;
            }

            this->unmake_move();

            if (eval > current_best_value) 
            {
                current_best_value = eval;
                current_best_move = move;
            }

            if (time_limit_reached)
            {
                break;
            }
        }

        if (time_limit_reached)
        {
            depths_searched = depth;
            break;
        }

        bestValue = current_best_value;
        best_move = current_best_move;
    }

    std::cout << "Stopped while searching depth " << depths_searched << std::endl;
    std::cout << "Evaluation: " << bestValue << std::endl;
    return best_move;
}

uint64_t Board::count_legal_moves_at_depth(int depth)
{
    std::vector<Move> moves = this->get_legal_moves();
    uint64_t total_moves = 0;

    if (depth == 1)
    {
        return moves.size();
    }
    else
    {
        for (int i = 0; i < moves.size(); i++)
        {
            this->make_move(moves[i]);
            total_moves += count_legal_moves_at_depth(depth - 1);
            this->unmake_move();
        }
    }
    return total_moves;
}

bool is_mate_score(int value)
{
    if (abs(value - MATE_VALUE) < 1000)
    {
        return true;
    }
    return false;
}

int retrieve_correct_mate_score(int value, int depth)
{
    if (is_mate_score(value))
    {
        int sign = 0;
        if (value > 0)
            sign = 1;
        else if (value < 0)
            sign = -1;
        return (value * sign - depth) * sign;
    }
    return value;
}

int store_correct_mate_score(int value, int depth)
{
    if (is_mate_score(value))
    {
        int sign = 0;
        if (value > 0)
            sign = 1;
        else if (value < 0)
            sign = -1;
        //return (value * sign + depth) * sign;
    }
    return value;
}

void prioritize_best_move(std::vector<Move>& moves, const Move& bestMove) {
    std::partition(moves.begin(), moves.end(), [&bestMove](const Move& move) {
        return move == bestMove;
        });
}

void print_bitboard(uint64_t bitboard) 
{
    for (int rank = 7; rank >= 0; --rank) 
    {
        for (int file = 0; file < 8; ++file) 
        {
            uint64_t square = 1ULL << (rank * 8 + file);
            std::cout << (bitboard & square ? '1' : '0') << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

void load_data(uint64_t* values, std::string file_name)
{
    std::ifstream file("bitboards/" + file_name);

    if (!file.is_open()) 
    {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }
    std::string line;
    int index = 0;
    while (std::getline(file, line) && index < NUM_LINES) 
    {
        std::stringstream ss(line);
        ss >> std::hex >> values[index];
        ++index;
    }
    file.close();

    return;
}

void load_bitboards() 
{
    load_data(rook_moves, "rook_moves.txt");
    load_data(bishop_moves, "bishop_moves.txt");
    load_data(queen_moves, "queen_moves.txt");
    load_data(knight_moves, "knight_moves.txt");
    load_data(king_moves, "king_moves.txt");

    load_data(rook_moves_up, "rook_moves_up.txt");
    load_data(rook_moves_down, "rook_moves_down.txt");
    load_data(rook_moves_left, "rook_moves_left.txt");
    load_data(rook_moves_right, "rook_moves_right.txt");

    load_data(bishop_moves_up, "bishop_moves_up.txt");
    load_data(bishop_moves_down, "bishop_moves_down.txt");
    load_data(bishop_moves_left, "bishop_moves_left.txt");
    load_data(bishop_moves_right, "bishop_moves_right.txt");

    load_data(pawn_white_moves, "pawn_moves_white.txt");
    load_data(pawn_black_moves, "pawn_moves_black.txt");
    load_data(pawn_white_capture_moves, "pawn_capture_moves_white.txt");
    load_data(pawn_black_capture_moves, "pawn_capture_moves_black.txt");
}

std::vector<int> get_set_bit_indices(uint64_t board) 
{
    std::vector<int> indices;

    for (int i = 0; i < 64; ++i) 
    {
        if (board & (1ULL << i)) 
        {
            indices.push_back(i);
        }
    }

    return indices;
}

int findMostSignificantBitIndex(uint64_t value) 
{
    unsigned long index;
    if (_BitScanReverse64(&index, value)) 
    {
        return static_cast<int>(index);
    }
    return -1;
}
int findLeastSignificantBitIndex(uint64_t value) 
{
    unsigned long index;
    if (_BitScanForward64(&index, value)) 
    {
        return static_cast<int>(index);
    }
    return -1;
}

std::string get_square_name(int square)
{
    if (square < 0 || square > 63) 
    {
        return "";
    }

    char file = 'a' + (square % 8);
    int rank = 1 + (square / 8);

    return std::string(1, file) + std::to_string(rank);
}

int get_piece_value(int piece)
{
    int values[6] = { 100, 300, 300, 500, 900, 100000 };
    return values[piece];
}

bool compare_moves(const Move& a, const Move& b)
{
    bool result = (a.capture_value > b.capture_value);
    return result;
    
}

void initialize_zobrist_table()
{
    std::mt19937_64 rng(12345); // Seed for reproducibility
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    for (int square = 0; square < BOARD_SIZE; ++square) 
        for (int piece = 0; piece < PIECE_TYPES; ++piece) 
            zobrist_table[square][piece] = dist(rng);

    for (int i = 0; i < CASTLING_RIGHTS; ++i) 
        zobrist_castling[i] = dist(rng);

    for (int i = 0; i < EN_PASSANT; ++i) 
        zobrist_en_passant[i] = dist(rng);

    zobrist_side_is_black = dist(rng);
}
