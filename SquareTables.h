#pragma once
#ifndef SQUARETABLES_H
#define SQUARETABLES_H

int pawn_table[2][64] = 
{
    { //opening - middle game
         0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
         5,  5, 10, 25, 25, 10,  5,  5,
         0,  0,  0, 20, 20,  0,  0,  0,
         5, -5,-10,  0,  0,-10, -5,  5,
         5, 10, 10,-20,-20, 10, 10,  5,
         0,  0,  0,  0,  0,  0,  0,  0
    },
    { //late game
         0,  0,  0,  0,  0,  0,  0,  0,
       100, 90, 80, 80, 80, 80, 90,100,
        60, 55, 50, 50, 50, 50, 55, 60,
        35, 30, 30, 30, 30, 30, 30, 35,
        20, 20, 20, 20, 20, 20, 20, 20,
        10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10,
         0,  0,  0,  0,  0,  0,  0,  0
    }
};

int knight_table[2][64] =
{
    { //opening - middle game
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50
    },
    { //late game
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50
    }
};

int bishop_table[2][64] =
{
    { //opening - middle game
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10, 10,  0,  0,  0,  0, 10,-10,
        0  ,-10,-10,-10,-10,-10,-10,  0
    },
    { //late game
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10, 10,  0,  0,  0,  0, 10,-10,
        5  ,-10,-10,-10,-10,-10,-10,  5
    }
};

int rook_table[2][64] =
{
    { //opening - middle game
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0,  5,  5,  0,  0,  0
    },
    { //late game
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 30, 30, 30, 30, 30, 30,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0,  5,  5,  0,  0,  0
    }
};

int queen_table[2][64] =
{
    { //opening - middle game
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
          0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    },
    { //late game
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
         -5,  0,  5,  5,  5,  5,  0, -5,
          0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    }
};

int king_table[2][64] =
{
    { //opening - middle game
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    },
    { //late game
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50
    }
};


int interpolate(int midgame, int endgame, int alpha, int scale = 1000)
{
    return endgame + ((alpha * (midgame - endgame)) / scale);
}

int get_polynomial_value(int value, float c0, float c1, float c2)
{
    float y = c2 * value * value + c1 * value + c0;

    if (y < 20)
        y = 0;
    else if (y > 980)
        y = 1000;

    int alpha = static_cast<int>(y);
    return alpha;
}

int get_piece_value(int piece)
{
    //pawn // knight // bishop // rook // queen // king
    int values[6] = { 100, 290, 300, 500, 900, 100000 };
    return values[piece];
}

int correct_value_for_array(int square, bool reversed)
{
    if (reversed)
        return square;

    if (square < 0 || square > 63)
        return -1;

    int row = square / 8;
    int col = square % 8;
    int transformed = (7 - row) * 8 + col;

    return transformed;
}

int get_piece_square_value(int piece, int square, int pieces_value, bool reversed)
{
    int alpha = get_polynomial_value(pieces_value, -289.4f, 0.317f, -0.0000167f);

    square = correct_value_for_array(square, reversed);

    switch (piece)
    {
        case 0:
            return interpolate(pawn_table[0][square], pawn_table[1][square], alpha);
            break;
        case 1:
            return interpolate(knight_table[0][square], knight_table[1][square], alpha);
            break;
        case 2:
            return interpolate(bishop_table[0][square], bishop_table[1][square], alpha);
            break;
        case 3:
            return interpolate(rook_table[0][square], rook_table[1][square], alpha);
            break;
        case 4:
            return interpolate(queen_table[0][square], queen_table[1][square], alpha);
            break;
        case 5:
            return interpolate(king_table[0][square], king_table[1][square], alpha);
            break;
    }
    return 0;
}

int get_bishop_pair_value(int empty_squares)
{
    int alpha = get_polynomial_value(empty_squares, 4140.8, -132.6, 1.06);

    return interpolate(10, 40, alpha);
}



#endif
