#include "Board.h"
#include "MoveBitboards.h"
#include "Evaluation.h"



Board::Board()
    :white_pieces{ 0x000000000000FF00, 0x0000000000000042, 0x0000000000000024,
                    0x0000000000000081, 0x0000000000000008, 0x0000000000000010 },
    black_pieces{ 0x00FF000000000000, 0x4200000000000000, 0x2400000000000000,
                    0x8100000000000000, 0x0800000000000000, 0x1000000000000000 },
    en_passant(0x0000000000000000)
{
    now_searching_for = 0;
    anti_moves.reserve(40);
    repetition_table.reserve(100);
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

int Board::get_knight_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color) //0 = white, 1 = black
{
    uint64_t result;
    uint64_t captures;
    uint64_t all_white = combine_white();
    uint64_t all_black = combine_black();

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
    return __popcnt64(result);
}

int Board::get_king_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color) //0 = white, 1 = black
{
    uint64_t result;
    uint64_t captures;
    uint64_t all_white = combine_white();
    uint64_t all_black = combine_black();

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
    return __popcnt64(result);
}

int Board::get_pawn_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color) //0 = white, 1 = black
{
    uint64_t all_white = combine_white();
    uint64_t all_black = combine_black();
    uint64_t all_pieces = all_white | all_black;
    uint64_t captures;
    uint64_t forwards;
    uint64_t block_mask;

    if (color == 0)
    {
        captures = pawn_white_capture_moves[square] & (all_black | en_passant);
        forwards = (pawn_white_moves[square] | all_pieces) - all_pieces;

        if (square / 8 == 1)
        {
            block_mask = (0x0000000000FF0000 & all_pieces) << 8;
            forwards = (forwards | block_mask) - block_mask;
        }

    }
    else
    {
        captures = pawn_black_capture_moves[square] & (all_white | en_passant);
        forwards = (pawn_black_moves[square] | all_pieces) - all_pieces;

        if (square / 8 == 6)
        {
            block_mask = (0x0000FF0000000000 & all_pieces) >> 8;
            forwards = (forwards | block_mask) - block_mask;
        }
    }

    create_moves(or_moves, captures | forwards, square, all_pieces, color, 0);
    return __popcnt64(captures | forwards);
}

int Board::get_rook_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color, bool is_queen)
{
    uint64_t all_white = combine_white();
    uint64_t all_black = combine_black();
    uint64_t all_pieces = all_white | all_black;

    uint64_t* rook_moves[] = { rook_moves_up, rook_moves_down, rook_moves_left, rook_moves_right };
    uint64_t rook_dirs[4] = { rook_moves_up[square], rook_moves_down[square], rook_moves_left[square], rook_moves_right[square] };
    uint64_t rook_attacks[4];

    uint64_t blocker;
    int blocker_index;
    unsigned long index;
    for (int i = 0; i < 4; i++)
    {
        blocker = (all_pieces & rook_dirs[i]);
        if (i == 0 || i == 3)
            _BitScanForward64(&index, blocker);
        else
            _BitScanReverse64(&index, blocker);
        blocker_index = static_cast<int>(index);

        rook_attacks[i] = (blocker == 0) ? rook_dirs[i] : (rook_dirs[i] & ~rook_moves[i][blocker_index]);
    }

    uint64_t rook_attacks_all = rook_attacks[0] | rook_attacks[1] | rook_attacks[2] | rook_attacks[3];
    rook_attacks_all = color == 0 ? (rook_attacks_all & ~all_white) : (rook_attacks_all & ~all_black);
    uint64_t captures = color == 0 ? (rook_attacks_all & all_black) : (rook_attacks_all & all_white);

    create_moves(or_moves, rook_attacks_all, square, captures, color, 3 + is_queen);
    return __popcnt64(rook_attacks_all);
}

int Board::get_bishop_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color, bool is_queen)
{
    uint64_t all_white = combine_white();
    uint64_t all_black = combine_black();
    uint64_t all_pieces = all_white | all_black;

    uint64_t* bishop_moves[] = { bishop_moves_up, bishop_moves_down, bishop_moves_left, bishop_moves_right };
    uint64_t bishop_dirs[4] = { bishop_moves_up[square], bishop_moves_down[square], bishop_moves_left[square], bishop_moves_right[square] };
    uint64_t bishop_attacks[4];

    uint64_t blocker;
    int blocker_index;
    unsigned long index;
    for (int i = 0; i < 4; i++)
    {
        blocker = (all_pieces & bishop_dirs[i]);
        if (i == 0 || i == 3)
            _BitScanForward64(&index, blocker);
        else
            _BitScanReverse64(&index, blocker);
        blocker_index = static_cast<int>(index);

        bishop_attacks[i] = (blocker == 0) ? bishop_dirs[i] : (bishop_dirs[i] & ~bishop_moves[i][blocker_index]);
    }

    uint64_t bishop_attacks_all = bishop_attacks[0] | bishop_attacks[1] | bishop_attacks[2] | bishop_attacks[3];
    bishop_attacks_all = color == 0 ? (bishop_attacks_all & ~all_white) : (bishop_attacks_all & ~all_black);
    uint64_t captures = color == 0 ? (bishop_attacks_all & all_black) : (bishop_attacks_all & all_white);

    create_moves(or_moves, bishop_attacks_all, square, captures, color, 2 + 2 * is_queen);
    return __popcnt64(bishop_attacks_all);
}

int Board::get_queen_moves(bool add_to_vector, std::vector<Move>& or_moves, int square, bool color)
{
    int a = get_rook_moves(add_to_vector, or_moves, square, color, 1);
    int b = get_bishop_moves(add_to_vector, or_moves, square, color, 1);

    return a + b;
}

bool Board::if_check(bool color, int square)
{
    int king_square;
    if (square == -1)
    {
        unsigned long king_square_long;
        
        if (color == 0)
            _BitScanForward64(&king_square_long, white_pieces[5]);
        else
            _BitScanForward64(&king_square_long, black_pieces[5]);
        king_square = static_cast<int>(king_square_long);
    }
    else
        king_square = square;

    uint64_t knight_captures;
    uint64_t king_captures;
    uint64_t pawn_captures;
    uint64_t rook_captures;
    uint64_t bishop_captures;
    uint64_t result;
    uint64_t all_white = white_pieces[0] | white_pieces[1] | white_pieces[2] | white_pieces[3] | white_pieces[4] | white_pieces[5];//combine_white();
    uint64_t all_black = black_pieces[0] | black_pieces[1] | black_pieces[2] | black_pieces[3] | black_pieces[4] | black_pieces[5];//combine_black();
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

    uint64_t* rook_moves[] = { rook_moves_up, rook_moves_down, rook_moves_left, rook_moves_right };
    uint64_t rook_dirs[4] = { rook_moves_up[king_square], rook_moves_down[king_square], rook_moves_left[king_square], rook_moves_right[king_square] };
    uint64_t rook_attacks[4];

    uint64_t blocker;
    int blocker_index;
    unsigned long index;
    for (int i = 0; i < 4; i++)
    {
        blocker = (all_pieces & rook_dirs[i]);
        if (i == 0 || i == 3)
            _BitScanForward64(&index, blocker);
        else
            _BitScanReverse64(&index, blocker);
        blocker_index = static_cast<int>(index);

        rook_attacks[i] = (blocker == 0) ? rook_dirs[i] : (rook_dirs[i] & ~rook_moves[i][blocker_index]);
    }

    result = rook_attacks[0] | rook_attacks[1] | rook_attacks[2] | rook_attacks[3];
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

    uint64_t* bishop_moves[] = { bishop_moves_up, bishop_moves_down, bishop_moves_left, bishop_moves_right };
    uint64_t bishop_dirs[4] = { bishop_moves_up[king_square], bishop_moves_down[king_square], bishop_moves_left[king_square], bishop_moves_right[king_square] };
    uint64_t bishop_attacks[4];

    for (int i = 0; i < 4; i++)
    {
        blocker = (all_pieces & bishop_dirs[i]);
        if (i == 0 || i == 3)
            _BitScanForward64(&index, blocker);
        else
            _BitScanReverse64(&index, blocker);
        blocker_index = static_cast<int>(index);

        bishop_attacks[i] = (blocker == 0) ? bishop_dirs[i] : (bishop_dirs[i] & ~bishop_moves[i][blocker_index]);
    }

    result = bishop_attacks[0] | bishop_attacks[1] | bishop_attacks[2] | bishop_attacks[3];
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

void Board::clear()
{
    time_limit_reached = false;
    for (int i = 0; i < 6; i++)
    {
        white_pieces[i] = 0x0000000000000000;
        black_pieces[i] = 0x0000000000000000;
    }
    en_passant = 0;
    for (int i = 0; i < 4; i++)
    {
        castling[i] = 0;
    }
    
    anti_moves.clear();
    repetition_table.clear();
}

void Board::setup_board_from_fen(const std::string& fen)
{
    clear();

    std::istringstream fen_stream(fen);
    std::string board_part, active_color, cstling, en_pass;

    fen_stream >> board_part >> active_color >> cstling >> en_pass;

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
            case 'P': set_piece(square, 0); break;
            case 'N': set_piece(square, 1); break;
            case 'B': set_piece(square, 2); break;
            case 'R': set_piece(square, 3); break;
            case 'Q': set_piece(square, 4); break;
            case 'K': set_piece(square, 5); break;
            case 'p': set_piece(square, 6); break;
            case 'n': set_piece(square, 7); break;
            case 'b': set_piece(square, 8); break;
            case 'r': set_piece(square, 9); break;
            case 'q': set_piece(square, 10); break;
            case 'k': set_piece(square, 11); break;
            }
            file++;
        }
    }

    if (active_color == "w")
    {
        turn = 0;
    }
    else if (active_color == "b")
    {
        turn = 1;
    }

    for (char c : cstling)
    {
        switch (c)
        {
        case 'K': castling[0] = 1; break;
        case 'Q': castling[1] = 1; break;
        case 'k': castling[2] = 1; break;
        case 'q': castling[3] = 1; break;
        }
    }

    if (en_pass != "-")
    {
        int file = en_pass[0] - 'a';
        int rank = en_pass[1] - '1';
        int square = rank * 8 + file;
        en_passant = (1ULL << square);;
    }
    else
    {
        en_passant = 0;
    }
}

void Board::set_piece(int square, int type)
{
    if (type < 6)
    {
        white_pieces[type] |= (1ULL << square);
    }
    else
    {
        black_pieces[type - 6] |= (1ULL << square);
    }
}

void Board::create_moves(std::vector<Move>& or_moves, uint64_t dests_board, int from, uint64_t capture_mask, bool color, int piece_type)
{
    std::array<int, 64> dests;
    std::array<int, 64> captures;
    int count = 0;

    unsigned long en_passant_index;
    _BitScanForward64(&en_passant_index, en_passant);

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
            capture_value = 10 * get_piece_value_rigid(captured_piece.type) - get_piece_value_rigid(piece_type);
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
                uint64_t check = ((combine_white() | combine_black()) & ((1ULL << 5) | (1ULL << 6)));
                if (castling[0] && !if_check(0) && !if_check(0, 5) && !if_check(0, 6) && !check) {
                    or_moves.emplace_back(Move(from, 6, piece_type + color_bonus, 2));  // White king-side castling
                }
                check = ((combine_white() | combine_black()) & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3)));
                if (castling[1] && !if_check(0) && !if_check(0, 3) && !if_check(0, 2) && !check) {
                    or_moves.emplace_back(Move(from, 2, piece_type + color_bonus, 2));  // White queen-side castling
                }
            }
            else if (color == 1 && from == 60 && did_black_castles == 0) // Black king castling
            {
                did_black_castles = 1;
                uint64_t check = ((combine_white() | combine_black()) & ((1ULL << 61) | (1ULL << 62)));
                if (castling[2] && !if_check(1) && !if_check(1, 61) && !if_check(1, 62) && !check)
                {
                    or_moves.emplace_back(Move(from, 62, piece_type + color_bonus, 2));  // Black king-side castling
                }
                check = ((combine_white() | combine_black()) & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59)));
                if (castling[3] && !if_check(1) && !if_check(1, 59) && !if_check(1, 58) && !check) {
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

std::vector<Move> Board::get_legal_moves(bool check_legality, bool natural, bool color)
{
    if (natural == 0)
    {
        color = turn;
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

    for (int piece_type = 0; piece_type < 6; piece_type++) 
    {
        uint64_t bitboard = pieces[piece_type];
        while (bitboard) 
        {
            unsigned long index;
            _BitScanForward64(&index, bitboard);
            int square = static_cast<int>(index);

            switch (piece_type) 
            {
            case 0:
                get_pawn_moves(1, pseudo_legal, square, color);
                break;
            case 1:
                get_knight_moves(1, pseudo_legal, square, color);
                break;
            case 2:
                get_bishop_moves(1, pseudo_legal, square, color);
                break;
            case 3:
                get_rook_moves(1, pseudo_legal, square, color);
                break;
            case 4:
                get_queen_moves(1, pseudo_legal, square, color);
                break;
            case 5:
                get_king_moves(1, pseudo_legal, square, color);
                break;
            }
            bitboard &= bitboard - 1;
        }
    }

    if (check_legality)
    {
        int it = 0;
        while (it < pseudo_legal.size())
        {
            make_move(pseudo_legal[it]);
            if (if_check(!turn))
            {
                std::swap(pseudo_legal[it], pseudo_legal.back());
                pseudo_legal.pop_back();
                it--;
            }
            it++;

            unmake_move();
        }
    }

    return pseudo_legal;
}

void Board::make_move(Move move, bool reversible)
{
    if (reversible)
        anti_moves.emplace_back(AntiMove(white_pieces, black_pieces, en_passant, castling, turn));

    int from_square = move.from;
    int to_square = move.to;
    int piece_type = move.piece_type;
    bool color = piece_type < 6 ? 0 : 1;

    PieceAndBoard piece_before = get_piece_bitboard(from_square);
    PieceAndBoard captured_piece = get_piece_bitboard(to_square);
    int captured_from = to_square;
    int captured_type = captured_piece.type;
    bool cstl1 = castling[0];
    bool cstl2 = castling[1];
    bool cstl3 = castling[2];
    bool cstl4 = castling[3];
    uint64_t enpass = en_passant;
    en_passant = 0;

    uint64_t* piece_board = color == 0 ? &white_pieces[piece_type] : &black_pieces[piece_type - 6];

    if (move.type == 1 || move.type == 4 || move.type == 6 || move.type == 7 || move.type == 8) // Capture (pawn promotion can be a capture sometimes)
    {
        if (!reversible)
            repetition_table.clear();
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
        en_passant = (1ULL << en_passant_square);
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

    turn = !turn;

    repetition_table.emplace_back(DumbHash(white_pieces, black_pieces, en_passant, castling));
}

void Board::unmake_move()
{
    AntiMove move = anti_moves.back();
    anti_moves.pop_back();
    repetition_table.pop_back();

    std::copy(std::begin(move.white_pieces), std::end(move.white_pieces), std::begin(white_pieces));
    std::copy(std::begin(move.black_pieces), std::end(move.black_pieces), std::begin(black_pieces));
    std::copy(std::begin(move.castling), std::end(move.castling), std::begin(castling));

    turn = move.turn;
    en_passant = move.en_passant;
}

void Board::make_move_opponent(Move move)
{
    std::vector<Move> moves = this->get_legal_moves(0);

    for (int i = 0; i < moves.size(); i++)
    {
        if (moves[i].from == move.from && moves[i].to == move.to)
        {
            if (move.type == 4 || move.type == 6 || move.type == 7 || move.type == 8)
            {
                if (move.type == moves[i].type)
                {
                    this->make_move(moves[i], 0);
                    return;
                }
            }
            else
            {
                this->make_move(moves[i], 0);
                return;
            }
        }
    }
}

uint64_t Board::count_legal_moves_at_depth(int depth)
{
    std::vector<Move> moves = get_legal_moves(1);
    uint64_t total_moves = 0;

    if (depth == 1)
    {
        return moves.size();
    }
    else
    {
        for (int i = 0; i < moves.size(); i++)
        {
            make_move(moves[i]);
            total_moves += count_legal_moves_at_depth(depth - 1);
            unmake_move();
        }
    }
    return total_moves;
}


int Board::quiescence_search(int ply_from_root, int alpha, int beta)
{
    positions_evaluated += 1;
    int stand_pat = evaluate_position(this);

    if (stand_pat >= beta)
    {
        return beta;
    }
        
    if (alpha < stand_pat)
    {
        alpha = stand_pat;
    }

    std::vector<Move> moves = get_legal_moves(0);
    moves.erase(std::remove_if(moves.begin(), moves.end(), [](const Move& m) { return m.capture_value <= 0;  }), moves.end()); //remove noncaptures
    sort_moves(moves);

    int move_counter = 0;
    for (int i = 0; i < moves.size(); i++)
    {
        make_move(moves[i]);
        if (if_check(!turn)) // check move legality
        {
            unmake_move();
            continue;
        }
        move_counter += 1;

        int score = -quiescence_search(ply_from_root + 1, -beta, -alpha);

        unmake_move();

        if (score >= beta)
        {
            return beta;
        }

        if (score > alpha)
        {
            alpha = score;
        }

        if (std::chrono::steady_clock::now() >= end_time) {
            time_limit_reached = true;
            break;
        }
    }
    return alpha;
}


int Board::search(int ply_from_root, int depth_left, int alpha, int beta) {
    if (depth_left == 0 || time_limit_reached)
    {
        return quiescence_search(ply_from_root + 1, alpha, beta);
    }

    DumbHash dumb_hash = DumbHash(this->white_pieces, this->black_pieces, this->en_passant, this->castling);
    if (exists_more_than_once(repetition_table, dumb_hash))
    {
        if (now_searching_for == this->turn)
            return DRAW_VALUE;
        else
            return -DRAW_VALUE;
    }

    std::vector<Move> moves = get_legal_moves(1); // generate pseudo-legal moves

    if (moves.size() == 0)
    {
        if (if_check(turn))
            return -MATE_VALUE + ply_from_root;

        return 0;
    }

    sort_moves(moves);

    for (int i = 0; i < moves.size(); i++) 
    {
        make_move(moves[i]);
        int score = -search(ply_from_root + 1, depth_left - 1, -beta, -alpha);
        unmake_move();

        if (score >= beta)
        {
            return beta;
        }
            
        if (score > alpha)
        {
            alpha = score;
        }

        if (std::chrono::steady_clock::now() >= end_time) {
            time_limit_reached = true;
            break;
        }
    }

    return alpha;
}


Move Board::find_best_move_for_depth(int depth)
{
    now_searching_for = turn;
    time_limit_reached = false;
    timed_search = false;
    positions_evaluated = 0;

    std::vector<Move> moves = get_legal_moves(1);
    sort_moves(moves);
    Move best_move;
    int best_score = -INF;

    int alpha = -INF;
    int beta = INF;

    for (int i = 0; i < moves.size(); i++)
    {
        make_move(moves[i]);
        int score = -search(1, depth - 1, -beta, -alpha);
        unmake_move();

        if (score > best_score)
        {
            best_score = score;
            best_move = moves[i];
        }

        if (score > alpha)
        {
            alpha = score;
        }
    }

    std::cout << "info depth " << depth << " score cp " << best_score << " nodes " << positions_evaluated << "\n";
    return best_move;
}

Move Board::find_best_move_with_time_limit(int time_limit_ms)
{
    now_searching_for = turn;
    time_limit_reached = false;
    timed_search = true;
    auto start_time = std::chrono::steady_clock::now();
    end_time = start_time + std::chrono::milliseconds(time_limit_ms);

    positions_evaluated = 0;
    Move best_move;
    int best_score = -INF;

    std::vector<Move> moves = get_legal_moves(1);
    sort_moves(moves);

    for (int depth = 2; depth <= MAX_DEPTH; depth++)
    {
        int alpha = -INF;
        int beta = INF;
        Move current_best_move;

        for (int i = 0; i < moves.size(); i++)
        {
            make_move(moves[i]);
            int score = -search(1, depth - 1, -beta, -alpha);
            unmake_move();

            if (time_limit_reached)
                break;

            if (score > alpha)
            {
                alpha = score;
                current_best_move = moves[i];
            }

        }

        if (time_limit_reached)
        {
            if (alpha > best_score)
            {
                best_move = current_best_move;
                best_score = alpha;
            }
            std::cout << "info depth " << depth << " score cp " << best_score << " nodes " << positions_evaluated << "\n";
            break;
        }

        best_move = current_best_move;
        best_score = alpha;

        std::cout << "info depth " << depth << " score cp " << best_score << " nodes " << positions_evaluated << "\n";
    }

    return best_move;
}


void Board::sort_moves(std::vector<Move>& moves) {
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        return a.capture_value > b.capture_value;
    });
}

void Board::print_chessboard()
{
    const char* piece_symbols[12] = { "P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k" };

    std::cout << "+-----------------+\n";

    for (int rank = 7; rank >= 0; --rank)
    {
        std::cout << "| ";
        for (int file = 0; file < 8; ++file)
        {
            int square = rank * 8 + file;
            PieceAndBoard piece = get_piece_bitboard(square);

            if (piece.type == 5)
            {
                int x = 1;
            }

            if (piece.type == -1)
            {
                if ((rank + file) % 2 == 0) {
                    std::cout << ". ";  // Light square
                }
                else {
                    std::cout << ". ";  // Dark square
                }
            }
            else
            {
                std::cout << piece_symbols[piece.type] << " ";
            }
        }
        std::cout << "|\n";
    }
    std::cout << "+-----------------+\n\n";
}

bool exists_more_than_once(const std::vector<DumbHash>& vec, DumbHash value) {
    int count = 0;

    for (const auto& elem : vec)
    {
        if (elem == value)
        {
            count++;
            if (count > 1)
            {
                return true;
            }
        }
    }

    return false;
}
