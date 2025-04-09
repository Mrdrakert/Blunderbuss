#include "Board.h"
#include "MoveBitboards.h"
#include "RookMagic.h"
#include "BishopMagic.h"
#include <iostream>
#include <intrin.h>
#include <stdlib.h>
#include <sstream>
#include <vector>
#include <cstring>

// Inline helper to pop the least-significant 1 bit from a bitboard.
// Returns the index of the bit that was removed.
inline int pop_lsb(uint64_t& bb)
{
    unsigned long index;
    _BitScanForward64(&index, bb);
    bb &= bb - 1;
    return index;
}

// Unrolled occupancy function for one side (white if color==0, black if color==1)
inline uint64_t GetOccupancy(Board* board, bool color)
{
    // Instead of iterating over piece types in a loop,
    // unroll the bitwise OR to enable compiler optimizations.
    return board->pieces[color][0] |
        board->pieces[color][1] |
        board->pieces[color][2] |
        board->pieces[color][3] |
        board->pieces[color][4] |
        board->pieces[color][5];
}

// Initialize the board with starting positions
Board* InitBoard() {
    Board* board = new Board();
    LoadFEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    return board;
}

Snapshot MakeSnapshot(Board* board)
{
    Snapshot snap;
    snap.turn = board->turn;
    memcpy(snap.pieces, board->pieces, 2 * 6 * sizeof(uint64_t));
    memcpy(snap.castling, board->castling, 4 * sizeof(bool));
    snap.en_passant = board->en_passant;
    return snap;
}

// Print the chessboard
void PrintBoard(Board* board) 
{
    const char* pieceSymbols = "PNBRQKpnbrqk"; // white: PNBRQK, black: pnbrqk

    for (int rank = 7; rank >= 0; --rank) 
    { // Loop through ranks 8 to 1
        for (int file = 0; file < 8; ++file) 
        { // Loop through files a to h
            int square = rank * 8 + file;  // Calculate square index (0 to 63)
            char piece = '.';  // Default empty square

            // Check for a piece on this square from either side
            for (int i = 0; i < 2; ++i) 
            {
                for (int j = 0; j < 6; ++j) 
                {
                    if (((board->pieces[i][j] >> square) & 1ULL) != 0) 
                    {
                        piece = pieceSymbols[i * 6 + j];
                        goto print_square;  // Found a piece; skip checking remaining pieces
                    }
                }
            }
        print_square:
            std::cout << piece << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}


uint64_t GetRookMoves(Board* board, int square, bool color)
{
    uint64_t occupancy = GetOccupancy(board, color) | GetOccupancy(board, !color);
    uint64_t myOccupancy = GetOccupancy(board, color);
    uint64_t moves = 0;

    occupancy = rookMagicMask[square] & occupancy;
	uint64_t magicNumber = rookMagics[square];
	uint64_t occupancyIndex = (occupancy * magicNumber) >> (64 - rookMagicBits);
    
	moves = rookMagicTable[square][occupancyIndex];
	moves &= ~myOccupancy; // Remove squares occupied by friendly pieces

    return moves;
}


uint64_t GetBishopMoves(Board* board, int square, bool color)
{
    uint64_t occupancy = GetOccupancy(board, color) | GetOccupancy(board, !color);
    uint64_t myOccupancy = GetOccupancy(board, color);
    uint64_t moves = 0;

    occupancy = bishopMagicMask[square] & occupancy;
    uint64_t magicNumber = bishopMagics[square];
    uint64_t occupancyIndex = (occupancy * magicNumber) >> (64 - bishopMagicBits);

    moves = bishopMagicTable[square][occupancyIndex];
    moves &= ~myOccupancy; // Remove squares occupied by friendly pieces

    return moves;
}


uint64_t GetKnightMoves(Board* board, int square, bool color)
{
    uint64_t myOccupancy = GetOccupancy(board, color);
    return knight_moves[square] & ~myOccupancy;
}

uint64_t GetKingMoves(Board* board, int square, bool color)
{
    uint64_t myOccupancy = GetOccupancy(board, color);
    uint64_t opOccupancy = GetOccupancy(board, !color);

    uint64_t moves = king_moves[square] & ~myOccupancy;

    // Check for castling possibilities
    if (color == 0) // White
    {
        if (board->castling[0] && ((myOccupancy | opOccupancy) & 0x0000000000000060ULL) == 0)
            moves |= 0x0000000000000040ULL; // King-side castling
        if (board->castling[1] && ((myOccupancy | opOccupancy) & 0x000000000000000EULL) == 0)
            moves |= 0x0000000000000004ULL; // Queen-side castling
    }
    else // Black
    {
        if (board->castling[2] && ((myOccupancy | opOccupancy) & 0x6000000000000000ULL) == 0)
            moves |= 0x4000000000000000ULL; // King-side castling
        if (board->castling[3] && ((myOccupancy | opOccupancy) & 0x0E00000000000000ULL) == 0)
            moves |= 0x0400000000000000ULL; // Queen-side castling
    }

    return moves;
}

uint64_t GetPawnMoves(Board* board, int square, bool color, bool onlyCaptures)
{
    uint64_t moves = 0;
    uint64_t myOccupancy = GetOccupancy(board, color);
    uint64_t opOccupancy = GetOccupancy(board, !color);
    uint64_t occupancy = myOccupancy | opOccupancy;

    if (color == 0)
    {
        moves |= (opOccupancy | board->en_passant) & pawn_white_capture_moves[square];
        if (!onlyCaptures)
        {
            uint64_t whiteMask = 0x0000000000FF0000ULL;
            // For double advance, avoid interference by friendly pieces.
            if (square / 8 == 1)
                occupancy |= ((whiteMask & occupancy) << 8);
            moves |= ~occupancy & pawn_white_moves[square];
        }
    }
    else
    {
        moves |= (opOccupancy | board->en_passant) & pawn_black_capture_moves[square];
        if (!onlyCaptures)
        {
            uint64_t blackMask = 0x0000FF0000000000ULL;
            if (square / 8 == 6)
                occupancy |= ((blackMask & occupancy) >> 8);
            moves |= ~occupancy & pawn_black_moves[square];
        }
    }
    return moves;
}

std::vector<Move> GetMovesSide(Board* board, bool color)
{
    std::vector<Move> moves;
    moves.reserve(100);

    for (int pieceType = 0; pieceType < 6; pieceType++)
    {
        uint64_t pieces = board->pieces[color][pieceType];
        // Iterate over all pieces of this type using pop_lsb.
        while (pieces)
        {
            int fromSquare = pop_lsb(pieces);
            uint64_t pieceMoves = 0;
            switch (pieceType) {
            case 0:
                pieceMoves = GetPawnMoves(board, fromSquare, color, false);
                break;
            case 1:
                pieceMoves = GetKnightMoves(board, fromSquare, color);
                break;
            case 2:
                pieceMoves = GetBishopMoves(board, fromSquare, color);
                break;
            case 3:
                pieceMoves = GetRookMoves(board, fromSquare, color);
                break;
            case 4:
                // Queen moves: union of bishop and rook moves.
                pieceMoves = GetBishopMoves(board, fromSquare, color) | GetRookMoves(board, fromSquare, color);
                break;
            case 5:
                pieceMoves = GetKingMoves(board, fromSquare, color);
                break;
            }

            // Iterate over each destination square.
            uint64_t destinations = pieceMoves;
            while (destinations)
            {
                int toSquare = pop_lsb(destinations);
                Move move;
                move.from = fromSquare;
                move.to = toSquare;
                move.pieceType = pieceType;
                move.enPassantSquare = -1;
                move.special = 0;
                move.capturedPiece = 0;

                // Special handling for pawn promotions and en passant.
                if (pieceType == 0)
                {
                    if (toSquare / 8 == 7 || toSquare / 8 == 0)
                    {
                        move.special = 4; // promotion to queen
                        moves.push_back(move);
                        move.special = 5; // promotion to knight
                        moves.push_back(move);
                        move.special = 6; // promotion to rook
                        moves.push_back(move);
                        move.special = 7; // promotion to bishop
                        moves.push_back(move);
                    }
                    else if ((1ULL << toSquare) == board->en_passant)
                    {
                        move.special = 2; // en passant capture
                        moves.push_back(move);
                    }
                    else if (abs(toSquare - fromSquare) == 16)  // two squares pawn advance
                    {
                        move.enPassantSquare = (fromSquare + toSquare) / 2;
                        moves.push_back(move);
                    }
                    else
                    {
                        moves.push_back(move);
                    }
                }
                else if (pieceType == 5)  // King
                {
                    if (abs(toSquare - fromSquare) == 2)
                    {
                        move.special = 3; // castling move
                    }
                    moves.push_back(move);
                }
                else
                {
                    moves.push_back(move);
                }
            }
        }
    }
    return moves;
}


void MakeMove(Board* board, Move move)
{
    int color = board->turn ? 1 : 0;
    int opponentColor = 1 ^ color;
    int pieceType = move.pieceType;
    if (pieceType == -1) return;

    uint64_t fromMask = 1ULL << move.from;
    uint64_t toMask = 1ULL << move.to;

    // Remove piece from origin square.
    board->pieces[color][pieceType] ^= fromMask;

    // Handle captures (including en passant).
    uint64_t captureMask = toMask;
    if (move.special == 2)
    {
        captureMask = (opponentColor == 1) ? (toMask >> 8) : (toMask << 8);
    }
    for (int i = 0; i < 6; ++i)
    {
        if (board->pieces[opponentColor][i] & captureMask)
        {
            board->pieces[opponentColor][i] ^= captureMask;
            break; // only one piece captured per move
        }
    }

    // Update castling rights based on rook/king movement.
    static const int rookSquares[4] = { 0, 7, 56, 63 };
    for (int i = 0; i < 4; ++i)
    {
        if (move.from == rookSquares[i] || move.to == rookSquares[i])
            board->castling[i] = false;
    }

    // Handle castling move: move the rook accordingly.
    if (move.special == 3)
    {
        int rookFrom = (move.to > move.from) ? (move.from + 3) : (move.from - 4);
        int rookTo = (move.to > move.from) ? (move.to - 1) : (move.to + 1);
        uint64_t rookFromMask = 1ULL << rookFrom;
        uint64_t rookToMask = 1ULL << rookTo;
        board->pieces[color][3] ^= rookFromMask;
        board->pieces[color][3] |= rookToMask;
    }

    // If the king moved, remove both castling rights for that side.
    if (pieceType == 5)
    {
        board->castling[color * 2] = false;
        board->castling[color * 2 + 1] = false;
    }

    // Handle promotion.
    if (move.special >= 4 && move.special <= 7)
    {
        static const int promotionMap[] = { 4, 1, 3, 2 }; // Q, N, R, B respectively
        board->pieces[color][promotionMap[move.special - 4]] |= toMask;
    }
    else
    {
        board->pieces[color][pieceType] |= toMask;
    }

    // Set en passant square.
    board->en_passant = (move.enPassantSquare != -1) ? (1ULL << move.enPassantSquare) : 0;
    board->turn ^= 1;
}

void UnmakeMove(Board* board, Snapshot snap)
{
    board->turn = snap.turn;
    memcpy(board->pieces, snap.pieces, 2 * 6 * sizeof(uint64_t));
    memcpy(board->castling, snap.castling, 4 * sizeof(bool));
    board->en_passant = snap.en_passant;
}

bool IsCheck(Board* board, bool color, int square)
{
    int kingSquare = square;
    if (square == -1)
    {
        unsigned long index = 0;
        _BitScanForward64(&index, board->pieces[color][5]);
        kingSquare = index;
    }

    uint64_t kingMoves = GetKingMoves(board, kingSquare, color);
    uint64_t rookMoves = GetRookMoves(board, kingSquare, color);
    uint64_t bishopMoves = GetBishopMoves(board, kingSquare, color);
    uint64_t knightMoves = GetKnightMoves(board, kingSquare, color);
    uint64_t pawnMoves = GetPawnMoves(board, kingSquare, color, true);

    if (rookMoves & (board->pieces[!color][3] | board->pieces[!color][4])) return true;
    if (bishopMoves & (board->pieces[!color][2] | board->pieces[!color][4])) return true;
    if (knightMoves & board->pieces[!color][1]) return true;
    if (pawnMoves & board->pieces[!color][0]) return true;
    if (kingMoves & board->pieces[!color][5]) return true;

    return false;
}

int PieceTypeFromLetter(char c)
{
    c = toupper(c);
    switch (c)
    {
    case 'P': return 0;
    case 'N': return 1;
    case 'B': return 2;
    case 'R': return 3;
    case 'Q': return 4;
    case 'K': return 5;
    default: return -1;
    }
}

void LoadFEN(Board* board, const std::string& fen)
{
    memset(board->pieces, 0, sizeof(board->pieces));
    board->turn = 0;
    board->en_passant = 0;

    std::istringstream iss(fen);
    std::string position;
    iss >> position;
    int rank = 7;
    int file = 0;
    for (char c : position)
    {
        if (c >= '1' && c <= '8')
        {
            file += c - '0';
        }
        else if (c == '/')
        {
            rank--;
            file = 0;
        }
        else
        {
            int color = isupper(c) ? 0 : 1;
            int pieceType = PieceTypeFromLetter(c);
            board->pieces[color][pieceType] |= (1ULL << (rank * 8 + file));
            file++;
        }
    }
    std::string turn;
    iss >> turn;
    board->turn = (turn == "b") ? 1 : 0;
    std::string castling;
    iss >> castling;
    for (int i = 0; i < 4; ++i)
        board->castling[i] = false;
    for (char c : castling)
    {
        switch (c)
        {
        case 'K': board->castling[0] = true; break;
        case 'Q': board->castling[1] = true; break;
        case 'k': board->castling[2] = true; break;
        case 'q': board->castling[3] = true; break;
        }
    }
    std::string enPassant;
    iss >> enPassant;
    if (enPassant != "-")
    {
        int file = enPassant[0] - 'a';
        int rank = enPassant[1] - '1';
        board->en_passant = (1ULL << (rank * 8 + file));
    }
}

// Converts a move to a string in algebraic coordinate notation (e.g. e2e4)
std::string MoveToString(Move move)
{
    std::string result;
    result.push_back('a' + (move.from % 8));
    result.push_back('1' + (move.from / 8));
    result.push_back('a' + (move.to % 8));
    result.push_back('1' + (move.to / 8));
    return result;
}

bool IsMoveLegal(Board* board, Move move)
{
    // For castling, check that none of the squares the king travels through are attacked.
    if (move.special == 3)
    {
        if (IsCheck(board, !board->turn, move.to))     return false;
        else if (IsCheck(board, !board->turn, move.from)) return false;
        else if (IsCheck(board, !board->turn, (move.to + move.from) / 2))
            return false;
        return true;
    }
    else
    {
        return !IsCheck(board, !board->turn);
    }
}

uint64_t Perft(Board* board, int depth, bool initial)
{
    if (depth == 0)
        return 1;

    uint64_t nodes = 0;
    std::vector<Move> moves = GetMovesSide(board, board->turn);

    for (const Move& move : moves)
    {
        Snapshot snap = MakeSnapshot(board);
        MakeMove(board, move);

        if (!IsMoveLegal(board, move))
        {
            UnmakeMove(board, snap);
            continue;
        }

        uint64_t temp = Perft(board, depth - 1, false);
        if (initial)
            std::cout << MoveToString(move) << " " << temp << "\n";
        nodes += temp;

        UnmakeMove(board, snap);
    }
    return nodes;
}
