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
	bool castling[4]; // 0 - white king side, 1 - white queen side, 2 - black king side, 3 - black queen side
    bool turn; // 0 - white, 1 - black
};

struct Snapshot
{
    uint64_t pieces[2][6];
    uint64_t en_passant;
    bool castling[4];
    bool turn;
};

struct Move
{
    int from;
    int to;
	int pieceType; // 0 - pawn, 1 - knight, 2 - bishop, 3 - rook, 4 - queen, 5 - king
	int special; // 0 - normal, 2 - en passant, 3 - castling, 4 - promotion to queen, 5 - promotion to knight, 6 - promotion to rook, 7 - promotion to bishop
    int capturedPiece; // exaple: 204, captured a queen with bishop
	int enPassantSquare; // -1 is no en passant
};

struct MoveScore
{
    Move move;
    int score;
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

bool IsMoveLegal(Board* board, Move move);

bool IsCheck(Board* board, bool color, int square = -1);

int PieceTypeFromLetter(char c);

std::string MoveToString(Move move);

int EvaluatePos(Board* board); 

int Search(Board* board, int depth, int alpha, int beta);

MoveScore SearchRoot(Board* board, int depth);

#endif // BOARD_H