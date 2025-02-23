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

    static void setWinLog(bool enable) { m_winLog = enable; }

private:
    enum class Level
    {
        INFO,
        WARN,
        LERROR,
    };

    static void log(const std::string &msg, Level level);
    static void consoleLog(const std::string &msg, Level level);
    static void winLog(const std::string &msg, Level level);

    static bool m_winLog;

};

} // namespace core