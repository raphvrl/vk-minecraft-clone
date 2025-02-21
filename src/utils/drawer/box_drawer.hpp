#pragma once

#include <iostream>
#include <sstream>

#include "core/types.hpp"

namespace utils
{

class BoxDrawer
{

public:
    static void draw(const std::string &content, usize width);
    static void draw(
        const std::string &top,
        const std::string &bottom,
        usize width
    );

private:
    static void drawTop(usize width);
    static void drawBottom(usize width);
    static void drawMiddle(usize width);

    static void drawContent(const std::string &content, usize width);

};

} // namespace utils