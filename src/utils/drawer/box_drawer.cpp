#include "box_drawer.hpp"

namespace utils
{

void BoxDrawer::draw(const std::string &content, usize width)
{
    drawTop(width);
    drawContent(content, width);
    drawBottom(width);
}

void BoxDrawer::draw(const std::string &top, const std::string &bottom, usize width)
{
    drawTop(width);
    drawContent(top, width);
    drawMiddle(width);
    drawContent(bottom, width);
    drawBottom(width);
}

void BoxDrawer::drawTop(usize width)
{
    std::cout << "┌";
    for (usize i = 0; i < width - 2; i++) {
        std::cout << "─";
    }

    std::cout << "┐" << std::endl;
}

void BoxDrawer::drawBottom(usize width)
{
    std::cout << "└";
    for (usize i = 0; i < width - 2; i++) {
        std::cout << "─";
    }

    std::cout << "┘" << std::endl;
}

void BoxDrawer::drawMiddle(usize width)
{
    std::cout << "├";
    for (usize i = 0; i < width - 2; i++) {
        std::cout << "─";
    }

    std::cout << "┤" << std::endl;
}

void BoxDrawer::drawContent(const std::string &content, usize width)
{
    std::istringstream stream(content);
    std::string word;
    std::string line;

    while (stream >> word) {
        if (line.length() + word.length() + (line.empty() ? 0 : 1) > width - 4) {
            std::cout << "\u2502 " << line 
                     << std::string(width - line.length() - 4, ' ')
                     << " \u2502" << std::endl;
            line = word;
        } else {
            if (!line.empty()) {
                line += " ";
            }
            line += word;
        }
    }

    if (!line.empty()) {
        std::cout << "\u2502 " << line 
                 << std::string(width - line.length() - 4, ' ')
                 << " \u2502" << std::endl;
    }
}

} // namespace utils