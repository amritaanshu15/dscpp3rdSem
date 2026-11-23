/*
 * Utils.h  -  small helpers
 * CPP topics: function template, exception class
 */
#ifndef UTILS_H
#define UTILS_H

#include <string>
using namespace std;

// function template: keeps a value between low and high
// works for int, double ... (any type that has < and >)
template <class T>
T clampValue(T value, T low, T high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

// our own exception class. We THROW it when the player does something wrong
// (for example moves after game over) and CATCH it in the server.
class GameError {
    string message;
public:
    GameError(string msg) { message = msg; }
    string what() const { return message; }
};

#endif
