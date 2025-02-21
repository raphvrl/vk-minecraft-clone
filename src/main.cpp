#include "core/window/window.hpp"
#include "utils/drawer/box_drawer.hpp"
#include "utils/console/console.hpp"

#ifdef _WIN32
#include <windows.h>
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

    core::Window window;

    window.init(800, 600, "Vulkan");

    while (window.isOpen()) {
        window.update();
    }

    window.destroy();

    return 0;
}
