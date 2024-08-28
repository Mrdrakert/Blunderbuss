#ifndef MOVE_H
#define MOVE_H


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

#endif  // MOVE_H