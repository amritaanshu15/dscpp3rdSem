/*
 * Pos.h  -  position (row, col) of an animal on the map
 * CPP topics: class, constructors (default + parameterized), this pointer,
 *             operator overloading (==, !=, +), friend function (<<), inline function
 */
#ifndef POS_H
#define POS_H

#include <iostream>
#include <cstdlib>
using namespace std;

class Pos {
public:
    int r;
    int c;

    Pos() { r = 0; c = 0; }                          // default constructor
    Pos(int r, int c) { this->r = r; this->c = c; }  // parameterized (this pointer)

    bool operator==(const Pos& other) const { return r == other.r && c == other.c; }
    bool operator!=(const Pos& other) const { return !(*this == other); }

    // Pos(2,3) + Pos(-1,0) = Pos(1,3)   (used for moving in a direction)
    Pos operator+(const Pos& other) const { return Pos(r + other.r, c + other.c); }

    // friend so it can print   cout << pos;
    friend ostream& operator<<(ostream& out, const Pos& p)
    {
        out << "(" << p.r << "," << p.c << ")";
        return out;
    }
};

// number of up/down/left/right steps between two cells (ignores walls)
inline int manhattan(Pos a, Pos b)
{
    return abs(a.r - b.r) + abs(a.c - b.c);
}

// distance when diagonal moves are allowed (used for eagle)
inline int chebyshev(Pos a, Pos b)
{
    int x = abs(a.r - b.r);
    int y = abs(a.c - b.c);
    return x > y ? x : y;
}

#endif
