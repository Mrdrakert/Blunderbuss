#pragma once
#ifndef BOARD_H
#define BOARD_H

#include <cstdint> // For uint64_t
#include <vector>

struct Board
{
    uint64_t pieces[2][6]; // 2 sides (white and black), 6 piece types each
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

uint64_t GetPawnMoves(Board* board, int square, bool color);

std::vector<Move> GetMovesSide(Board* board, bool color);

void MakeMove(Board* board, Move move);

void UnmakeMove(Board* board, Snapshot snap);

uint64_t Perft(Board* board, int depth);

#endif // BOARD_H