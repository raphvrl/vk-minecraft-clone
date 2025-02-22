#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "utils/drawer/box_drawer.hpp"
#include "utils/console/console.hpp"

namespace core
{

class Logger
{

public:
    static void info(const std::string &msg);
    static void warn(const std::string &msg);
    static void error(const std::string &msg);

private:
    enum class Level
    {
        INFO,
        WARN,
        LERROR,
    };

    static void log(const std::string &msg, Level level);

};

} // namespace core