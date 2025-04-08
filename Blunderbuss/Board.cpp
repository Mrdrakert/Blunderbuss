#include "Board.h"
#include "MoveBitboards.h"
#include <iostream>
#include <intrin.h>
#include <stdlib.h>
#include <sstream>

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
	uint64_t opOccupancy = GetOccupancy(board, !color);

    uint64_t result = ~myOccupancy & king_moves[square];
    moves |= result;

	// Check for castling
    if (color == 0) // White king
    {
        if (board->castling[0] && ((myOccupancy | opOccupancy) & 0x0000000000000060) == 0)
        {
            moves |= 0x0000000000000040; // King-side castling
        }
		if (board->castling[1] && ((myOccupancy | opOccupancy) & 0x000000000000000E) == 0)
		{
			moves |= 0x0000000000000004; // Queen-side castling
		}
    }
    else // Black king
    {
		if (board->castling[2] && ((myOccupancy | opOccupancy) & 0x6000000000000000) == 0)
		{
			moves |= 0x4000000000000000; // King-side castling
		}
        if (board->castling[3] && ((myOccupancy | opOccupancy) & 0x0E00000000000000) == 0)
        {
			moves |= 0x0400000000000000; // Queen-side castling
        }
    }

    return moves;
}

uint64_t GetPawnMoves(Board* board, int square, bool color, bool onlyCaptures)
{
    uint64_t whiteMask = 0x0000000000FF0000;
    uint64_t blackMask = 0x0000FF0000000000;

    uint64_t moves = 0;

    uint64_t myOccupancy = GetOccupancy(board, color);
    uint64_t opOccupancy = GetOccupancy(board, !color);
    uint64_t occupancy = myOccupancy | opOccupancy;

    if (color == 0)
    {
        moves |= (opOccupancy | board->en_passant) & pawn_white_capture_moves[square];
        if (onlyCaptures == false)
        {
            if (square / 8 == 1)
                occupancy |= ((whiteMask & occupancy) << 8);
            moves |= ~occupancy & pawn_white_moves[square];
        }
    }
    else
    {
        moves |= (opOccupancy | board->en_passant) & pawn_black_capture_moves[square];
        if (onlyCaptures == false)
        {
            if (square / 8 == 6)
                occupancy |= ((blackMask & occupancy) >> 8);
            moves |= ~occupancy & pawn_black_moves[square];
        }
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
                theMoves = GetPawnMoves(board, index, color, false);
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
				Move move = { static_cast<int>(index), static_cast<int>(index2) };
                move.pieceType = i;
                move.enPassantSquare = -1;
                move.special = 0;
                move.capturedPiece = 0;
                if (i == 0) // if the piece is a pawn
				{
					if (index2 / 8 == 7 || index2 / 8 == 0)
					{
						move.special = 4; // promotion to queen
						moves.push_back(move);
						move.special = 5; // promotion to knight
						moves.push_back(move);
						move.special = 6; // promotion to rook
						moves.push_back(move);
						move.special = 7; // promotion to bishop
						//moves.push_back(move);
					}
					else if ((1ULL << index2) == board->en_passant)
					{
						move.special = 2; // en passant
					}
					else if (abs(static_cast<int>(index2 - index)) == 16)  //if it moved 2 squares forward, set enpassant square
					{
						int square = (index + index2) / 2;
                        move.enPassantSquare = square;
					}

				}
				else if (i == 5) // castling
				{
					if (index2 == index + 2 || index2 == index - 2)
					{
						move.special = 3; // castling
					}
				}

                moves.push_back(move);
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
    int pieceType = move.pieceType;

    if (pieceType == -1)
    {
        return;
    }

    // Remove the piece from the 'from' square
    board->pieces[color][pieceType] &= ~(1ULL << move.from);

	uint64_t toSquare = (1ULL << move.to);
    // Check if the 'to' square is occupied by an opponent's piece (capture)
    for (int i = 0; i < 6; ++i)
    {
        if (board->pieces[opponentColor][i] & toSquare)
        {
            // Remove the captured piece
            board->pieces[opponentColor][i] &= ~toSquare;
            break;
        }
    }

    // If move type is enpassant, then remove the piece from the square behind
    if (move.special == 2)
    {
        int square = (opponentColor == 1) ? move.to - 8 : move.to + 8;
        if (board->pieces[opponentColor][0] & (1ULL << square))
        {
            // Remove the captured piece
            board->pieces[opponentColor][0] &= ~(1ULL << square);
        }
    }

    // if any rook is moved or something moves to its square, cancel that rook's castling
    if (move.to == 0 || move.from == 0)
        board->castling[1] = false;
    if (move.to == 7 || move.from == 7)
        board->castling[0] = false;

    if (move.to == 56 || move.from == 56)
        board->castling[3] = false;
    if (move.to == 63 || move.from == 63)
        board->castling[2] = false;

    // Check if the move is a castling move
    if (move.special == 3)
    {
        // Determine the direction of the castling
        if (move.to == move.from + 2) // King-side castling
        {
            // Move the rook to the appropriate square
            int rookFrom = move.from + 3;
            int rookTo = move.to - 1;
            board->pieces[color][3] &= ~(1ULL << rookFrom);
            board->pieces[color][3] |= (1ULL << rookTo);
        }
        else if (move.to == move.from - 2) // Queen-side castling
        {
            // Move the rook to the appropriate square
            int rookFrom = move.from - 4;
            int rookTo = move.to + 1;
            board->pieces[color][3] &= ~(1ULL << rookFrom);
            board->pieces[color][3] |= (1ULL << rookTo);
        }
    }

    if (pieceType == 5)
    {
        if (color == 0)
        {
            board->castling[0] = false; // White king-side castling
            board->castling[1] = false; // White queen-side castling
        }
        else
        {
            board->castling[2] = false; // Black king-side castling
            board->castling[3] = false; // Black queen-side castling
        }
    }

    if (move.special == 4)
    {
		board->pieces[color][4] |= toSquare;
	}
	else if (move.special == 5)
	{
		// Promote to knight
		board->pieces[color][1] |= toSquare;
	}
	else if (move.special == 6)
	{
		// Promote to rook
		board->pieces[color][3] |= toSquare;
	}
    else if (move.special == 7)
    {
        // Promote to bishop
        board->pieces[color][2] |= toSquare;
    }
    else
    {
        // Place the piece on the 'to' square
        board->pieces[color][pieceType] |= toSquare;
    }

    board->en_passant = (move.enPassantSquare != -1) ? (1ULL << move.enPassantSquare) : 0;

    board->turn = !board->turn;
}

void UnmakeMove(Board* board, Snapshot snap)
{
    board->turn = snap.turn;
    memcpy(board->pieces, snap.pieces, 2 * 6 * sizeof(uint64_t));
	memcpy(board->castling, snap.castling, 4 * sizeof(bool));
	board->en_passant = snap.en_passant;

    //delete snap;
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

	if (rookMoves & (board->pieces[!color][3] | board->pieces[!color][4]))
	{
		return true;
	}
	if (bishopMoves & (board->pieces[!color][2] | board->pieces[!color][4]))
	{
		return true;
	}
	if (knightMoves & board->pieces[!color][1])
	{
		return true;
	}
	if (pawnMoves & board->pieces[!color][0])
	{
		return true;
	}
    if (kingMoves & board->pieces[!color][5])
    {
        return true;
    }

    return false;
}

int PieceTypeFromLetter(char c)
{
	c = toupper(c); // Convert to uppercase
	switch (c)
	{
	    case 'P': return 0; // Pawn
	    case 'N': return 1; // Knight
	    case 'B': return 2; // Bishop
	    case 'R': return 3; // Rook
	    case 'Q': return 4; // Queen
	    case 'K': return 5; // King
	    default: return -1; // Invalid piece type
	}
}

void LoadFEN(Board* board, const std::string& fen)
{
    // Clear the board
    memset(board->pieces, 0, sizeof(board->pieces));
    board->turn = 0;
    board->en_passant = 0;
    // Parse the FEN string
    std::istringstream iss(fen);
    std::string position;
    iss >> position;
    int rank = 7;
    int file = 0;
    for (char c : position)
    {
        if (c >= '1' && c <= '8')
        {
            file += c - '0'; // Skip squares
        }
        else if (c == '/')
        {
            rank--;
            file = 0; // Reset file for the next rank
        }
        else
        {
            int color = isupper(c) ? 0 : 1; // Determine color (white or black)
            int pieceType = PieceTypeFromLetter(c); // Convert piece character to index
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
	{
		board->castling[i] = false;
	}
	for (char c : castling)
	{
		switch (c)
		{
		    case 'K': board->castling[0] = true; break; // White king side
		    case 'Q': board->castling[1] = true; break; // White queen side
		    case 'k': board->castling[2] = true; break; // Black king side
		    case 'q': board->castling[3] = true; break; // Black queen side
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


//function that returns a string with move notation, for example e2e4
std::string MoveToString(Move move)
{
	std::string result;
	char file = 'a' + (move.from % 8);
	char rank = '1' + (move.from / 8);
	result += file;
	result += rank;
	file = 'a' + (move.to % 8);
	rank = '1' + (move.to / 8);
	result += file;
	result += rank;
	return result;
}

bool IsMoveLegal(Board* board, Move move)
{
	if (move.special == 3)
	{

		if (IsCheck(board, !board->turn, move.to))
		{
			return false;
		}
        else if (IsCheck(board, !board->turn, move.from))
        {
            return false;
        }
        else if (IsCheck(board, !board->turn, (move.to + move.from) / 2))
        {
            return false;
        }
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
    {
        return 1;
    }

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