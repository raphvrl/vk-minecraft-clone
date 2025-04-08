#include "game/game.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#include <locale.h>
#endif

int main()
{

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#else
    setlocale(LC_ALL, ".UTF-8");
#endif

// to justify a commit message

    try {
        game::Game game;
        game.init();
        game.run();
        game.destroy();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}