#include "console.hpp"

namespace utils
{

void Console::setColor(Color color)
{

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, static_cast<int>(color));
#else
    std::printf("\033[%dm", static_cast<int>(color));
#endif

}

void Console::resetColor()
{

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7);
#else
    std::printf("\033[0m");
#endif

}

usize Console::getWidth()
{

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return w.ws_col;
#endif

}

} // namespace utils
