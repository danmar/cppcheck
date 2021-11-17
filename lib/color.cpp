#include "color.h"
#ifndef _WIN32
#include <unistd.h>
#endif
#include <sstream>

#ifdef _WIN32
std::ostream& operator<<(std::ostream& os, const Color& /*c*/)
{
#else
std::ostream& operator<<(std::ostream & os, const Color& c)
{
    static const bool use_color = isatty(STDOUT_FILENO);
    if (use_color)
        return os << "\033[" << static_cast<std::size_t>(c) << "m";
#endif
    return os;
}

std::string toString(const Color& c)
{
    std::stringstream ss;
    ss << c;
    return ss.str();
}

