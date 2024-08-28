#include "Move.h"


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
    return from == other.from && to == other.to && piece_type == other.piece_type && type == other.type && capture_value == other.capture_value; // Add other necessary comparisons
}