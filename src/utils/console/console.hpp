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
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE,
    RESET
};

class Console
{

public:
    static void setColor(Color color);
    static void resetColor();

    static usize getWidth();

};

} // namespace utils
