#include "logger.hpp"

namespace core
{

void Logger::info(const std::string &msg)
{
    log(msg, Level::INFO);
}

void Logger::warn(const std::string &msg)
{
    log(msg, Level::WARN);
}

void Logger::error(const std::string &msg)
{
    log(msg, Level::LERROR);
}

void Logger::log(const std::string &msg, Level level)
{

    if (m_winLog) {
        #ifdef _WIN32
        winLog(msg, level);
        #else
        consoleLog(msg, level);
        #endif
    } else {
        consoleLog(msg, level);
    }
    
}

void Logger::consoleLog(const std::string &msg, Level level)
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    usize width = utils::Console::getWidth();
    std::stringstream ss;

    utils::Color color = utils::Color::WHITE;
    std::string levelStr;

    switch (level) {
        case Level::INFO:
            color = utils::Color::BRIGHT_GREEN;
            levelStr = "INFO";
            break;
        case Level::WARN:
            color = utils::Color::BRIGHT_YELLOW;
            levelStr = "WARN";
            break;
        case Level::LERROR:
            color = utils::Color::BRIGHT_RED;
            levelStr = "ERROR";
            break;
    }

    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << " | ";
    ss << levelStr;

    utils::Console::setColor(color);
    utils::BoxDrawer::draw(ss.str(), msg, width);
    utils::Console::resetColor();

    std::cout << std::endl;
}

void Logger::winLog(const std::string &msg, Level level)
{
#ifdef _WIN32
    std::string title;

    switch (level) {
        case Level::INFO:
            title = "Info";
            break;
        case Level::WARN:
            title = "Warning";
            break;
        case Level::LERROR:
            title = "Error";
            break;
    }

    MessageBoxA(NULL, msg.c_str(), title.c_str(), MB_OK);
#endif

}

bool Logger::m_winLog = false;

} // namespace core