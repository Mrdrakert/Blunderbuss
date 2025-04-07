#pragma once
#ifndef BOARD_H
#define BOARD_H

#include <cstdint> // For uint64_t
#include <vector>
#include <string>

struct Board
{
    uint64_t pieces[2][6]; // 2 sides (white and black), 6 piece types each
	uint64_t en_passant; // En passant target square;
    bool turn; // 0 - white, 1 - black
};

struct Snapshot
{
    uint64_t pieces[2][6];
    bool turn;
};

struct Move
{
    int from;
    int to;
	int special; // 0 - normal, 1 - capture, 2 - en passant, 3 - castling, 4 - promotion to queen, 5 - promotion to knight, 6 - promotion to rook, 7 - promotion to bishop
    int capturedPiece; // exaple: 204, captured a queen with bishop
	int enPassantSquare; // -1 is no en passant
};

// Function to initialize the board
Board* InitBoard();

Snapshot MakeSnapshot(Board* board);

// Function to print the board
void PrintBoard(Board* board);

uint64_t GetOccupancy(Board* board, bool color);

uint64_t GetRookMoves(Board* board, int square, bool color);

uint64_t GetBishopMoves(Board* board, int square, bool color);

uint64_t GetKnightMoves(Board* board, int square, bool color);

uint64_t GetKingMoves(Board* board, int square, bool color);

uint64_t GetPawnMoves(Board* board, int square, bool color, bool onlyCaptures);

std::vector<Move> GetMovesSide(Board* board, bool color);

void MakeMove(Board* board, Move move);

void UnmakeMove(Board* board, Snapshot snap);

uint64_t Perft(Board* board, int depthm, bool initial);

void LoadFEN(Board* board, const std::string& fen);

int PieceTypeFromLetter(char c);

#endif // BOARD_H