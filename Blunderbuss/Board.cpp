#include "Board.h"
#include "MoveBitboards.h"
#include <iostream>
#include <intrin.h>

// Initialize the board with starting positions
Board* InitBoard() {
    Board* board = new Board();

    // White's turn
    board->turn = 0;

    // White pieces: pawn, knight, bishop, rook, queen, king
    board->pieces[0][0] = 0x000000000000FF00; // Pawns
    //board->pieces[0][0] = 0x0000000000000000; // Pawns
    board->pieces[0][1] = 0x0000000000000042; // Knights
    board->pieces[0][2] = 0x0000000000000024; // Bishops
    board->pieces[0][3] = 0x0000000000000081; // Rooks
    board->pieces[0][4] = 0x0000000000000008; // Queen
    board->pieces[0][5] = 0x0000000000000010; // King

    // Black pieces: pawn, knight, bishop, rook, queen, king
    board->pieces[1][0] = 0x00FF000000000000; // Pawns
    board->pieces[1][1] = 0x4200000000000000; // Knights
    board->pieces[1][2] = 0x2400000000000000; // Bishops
    board->pieces[1][3] = 0x8100000000000000; // Rooks
    board->pieces[1][4] = 0x0800000000000000; // Queen
    board->pieces[1][5] = 0x1000000000000000; // King

    return board;
}

Snapshot MakeSnapshot(Board* board)
{
    Snapshot snap;
    snap.turn = board->turn;
    memcpy(snap.pieces, board->pieces, 2 * 6 * sizeof(uint64_t));
    return snap;
}

// Print the chessboard
void PrintBoard(Board* board) {
    const char* pieceSymbols = "PNBRQKpnbrqk"; // Symbols for pieces (white: PNBRQK, black: pnbrqk)

    for (int rank = 7; rank >= 0; --rank) { // Loop through ranks (8 to 1)
        for (int file = 0; file < 8; ++file) { // Loop through files (a to h)
            int square = rank * 8 + file; // Calculate the square index (0 to 63)
            char piece = '.'; // Default to empty square

            // Check each piece type to see if it occupies the current square
            for (int i = 0; i < 2; ++i) { // Loop through sides (white and black)
                for (int j = 0; j < 6; ++j) { // Loop through piece types
                    if ((board->pieces[i][j] >> square) & 1) { // Check if the bit for this square is set
                        piece = pieceSymbols[i * 6 + j]; // Assign the corresponding piece symbol
                        break; // Stop checking other pieces once a match is found
                    }
                }
            }
            std::cout << piece << " "; // Print the piece or empty square
        }
        std::cout << "\n"; // New line after each rank
    }
    std::cout << "\n";
}

uint64_t GetOccupancy(Board* board, bool color)
{
    uint64_t occupancy = 0;
    for (int j = 0; j < 6; ++j) { // Loop through all piece types
        occupancy |= board->pieces[color][j];
    }
    return occupancy;
}

uint64_t GetRookMoves(Board* board, int square, bool color)
{
    bool myColor = color;
    uint64_t moves = 0;

    // Get the occupancy bitboard (all occupied squares)
    uint64_t occupancy = GetOccupancy(board, color) | GetOccupancy(board, !color);
    uint64_t myOccupancy = GetOccupancy(board, color);

    uint64_t* arrays[] = { rook_moves_left, rook_moves_right, rook_moves_down, rook_moves_up };
    for (int i = 0; i < 4; i++)
    {
        uint64_t andResult = arrays[i][square] & occupancy;
        uint64_t mask = 0;
        unsigned long highestBit = 0;
        if (i % 2 == 0)
        {
            if (_BitScanReverse64(&highestBit, andResult))
                mask = (highestBit == 63) ? ~0ULL : (1ULL << (highestBit)) - 1;
        }
        else
        {
            if (_BitScanForward64(&highestBit, andResult))
                mask = (highestBit == 63) ? 0ULL : ~((1ULL << (highestBit + 1)) - 1);
        }

        uint64_t result = arrays[i][square] & ~mask;
        uint64_t finalResult = result & ~myOccupancy;
        moves |= finalResult;
    }

    return moves;
}

uint64_t GetBishopMoves(Board* board, int square, bool color)
{
    bool myColor = color;
    uint64_t moves = 0;

    // Get the occupancy bitboard (all occupied squares)
    uint64_t occupancy = GetOccupancy(board, color) | GetOccupancy(board, !color);
    uint64_t myOccupancy = GetOccupancy(board, color);

    uint64_t* arrays[] = { bishop_moves_left, bishop_moves_right, bishop_moves_down, bishop_moves_up };
    for (int i = 0; i < 4; i++)
    {
        uint64_t andResult = arrays[i][square] & occupancy;
        uint64_t mask = 0;
        unsigned long highestBit = 0;
        if (i % 2 == 0)
        {
            if (_BitScanReverse64(&highestBit, andResult))
                mask = (highestBit == 63) ? ~0ULL : (1ULL << (highestBit)) - 1;
        }
        else
        {
            if (_BitScanForward64(&highestBit, andResult))
                mask = (highestBit == 63) ? 0ULL : ~((1ULL << (highestBit + 1)) - 1);
        }

        uint64_t result = arrays[i][square] & ~mask;
        uint64_t finalResult = result & ~myOccupancy;
        moves |= finalResult;
    }

    return moves;
}

uint64_t GetKnightMoves(Board* board, int square, bool color)
{
    bool myColor = color;
    uint64_t moves = 0;

    // Get the occupancy bitboard (all occupied squares)
    uint64_t myOccupancy = GetOccupancy(board, color);

    uint64_t result = ~myOccupancy & knight_moves[square];
    moves |= result;

    return moves;
}

uint64_t GetKingMoves(Board* board, int square, bool color)
{
    bool myColor = color;
    uint64_t moves = 0;

    // Get the occupancy bitboard (all occupied squares)
    uint64_t myOccupancy = GetOccupancy(board, color);

    uint64_t result = ~myOccupancy & king_moves[square];
    moves |= result;

    return moves;
}

uint64_t GetPawnMoves(Board* board, int square, bool color)
{
    uint64_t whiteMask = 0x0000000000FF0000;
    uint64_t blackMask = 0x0000FF0000000000;

    uint64_t moves = 0;

    uint64_t myOccupancy = GetOccupancy(board, color);
    uint64_t opOccupancy = GetOccupancy(board, !color);
    uint64_t occupancy = myOccupancy | opOccupancy;

    if (color == 0)
    {
        moves |= opOccupancy & pawn_white_capture_moves[square];
        if (square / 8 == 1)
            occupancy |= ((whiteMask & occupancy) << 8);
        moves |= ~occupancy & pawn_white_moves[square];
    }
    else
    {
        moves |= opOccupancy & pawn_black_capture_moves[square];
        if (square / 8 == 6)
            occupancy |= ((blackMask & occupancy) >> 8);
        moves |= ~occupancy & pawn_black_moves[square];
    }


    return moves;
}

std::vector<Move> GetMovesSide(Board* board, bool color)
{
    std::vector<Move> moves; // Vector to store moves
    moves.reserve(50);

    for (int i = 0; i < 6; i++)
    {
        uint64_t temp = board->pieces[color][i];

        unsigned long index = 0;
        while (_BitScanForward64(&index, temp))
        {
            uint64_t theMoves = 0;
            switch (i) {
            case 0:
                theMoves = GetPawnMoves(board, index, color);
                break;
            case 1:
                theMoves = GetKnightMoves(board, index, color);
                break;
            case 2:
                theMoves = GetBishopMoves(board, index, color);
                break;
            case 3:
                theMoves = GetRookMoves(board, index, color);
                break;
            case 4:
                theMoves = GetBishopMoves(board, index, color);
                theMoves |= GetRookMoves(board, index, color);
                break;
            case 5:
                theMoves = GetKingMoves(board, index, color);
                break;
            }

            unsigned long index2 = 0;
            while (_BitScanForward64(&index2, theMoves))
            {
                moves.push_back({ static_cast<int>(index), static_cast<int>(index2) });
                theMoves ^= (1ULL << index2);
            }
            temp ^= (1ULL << index);
        }
    }


    return moves; // Return the vector of moves
}


void MakeMove(Board* board, Move move)
{
    int color = board->turn ? 1 : 0;
    int opponentColor = 1 - color;
    int pieceType = -1;
    for (int i = 0; i < 6; ++i)
    {
        if (board->pieces[color][i] & (1ULL << move.from))
        {
            pieceType = i;
            break;
        }
    }

    if (pieceType == -1)
    {
        return;
    }

    // Remove the piece from the 'from' square
    board->pieces[color][pieceType] &= ~(1ULL << move.from);
    // Check if the 'to' square is occupied by an opponent's piece (capture)
    for (int i = 0; i < 6; ++i)
    {
        if (board->pieces[opponentColor][i] & (1ULL << move.to))
        {
            // Remove the captured piece
            board->pieces[opponentColor][i] &= ~(1ULL << move.to);
            break;
        }
    }

    // Place the piece on the 'to' square
    board->pieces[color][pieceType] |= (1ULL << move.to);

    board->turn = !board->turn;
}

void UnmakeMove(Board* board, Snapshot snap)
{
    board->turn = snap.turn;
    memcpy(board->pieces, snap.pieces, 2 * 6 * sizeof(uint64_t));

    //delete snap;
}

uint64_t Perft(Board* board, int depth)
{
    if (depth == 0)
    {
        return 1;
    }

    uint64_t nodes = 0;
    std::vector<Move> moves = GetMovesSide(board, board->turn);

    for (const Move& move : moves)
    {
        Snapshot snap = MakeSnapshot(board);
        MakeMove(board, move);

        nodes += Perft(board, depth - 1);

        UnmakeMove(board, snap);
    }

    return nodes;
}