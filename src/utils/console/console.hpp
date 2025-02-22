#pragma once

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdio>
#endif

#include "core/types.hpp"

namespace utils
{

enum class Color {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    YELLOW = 6,
    WHITE = 7,
    BRIGHT_BLACK = 8,
    BRIGHT_BLUE = 9,
    BRIGHT_GREEN = 10,
    BRIGHT_CYAN = 11,
    BRIGHT_RED = 12,
    BRIGHT_MAGENTA = 13,
    BRIGHT_YELLOW = 14,
    BRIGHT_WHITE = 15,
    RESET = 7
};

class Console
{

public:
    static void setColor(Color color);
    static void resetColor();

    static usize getWidth();

};

} // namespace utils
