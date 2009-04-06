#include "mathlib.h"


#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>

double MathLib::toNumber(const std::string &str)
{
    return atof(str.c_str());
}

std::string MathLib::toString(double d)
{
    std::ostringstream result;
    result << d;
    return result.str();
}

std::string MathLib::add(const std::string & first, const std::string & second)
{
    return toString(toNumber(first) + toNumber(second));
}

std::string MathLib::subtract(const std::string &first, const std::string &second)
{
    return toString(toNumber(first) - toNumber(second));
}

std::string MathLib::divide(const std::string &first, const std::string &second)
{
    return toString(toNumber(first) / toNumber(second));
}

std::string MathLib::multiply(const std::string &first, const std::string &second)
{
    return toString(toNumber(first) * toNumber(second));
}


std::string MathLib::sin(const std::string &tok)
{
    return toString(::sin(toNumber(tok)));
}


std::string MathLib::cos(const std::string &tok)
{
    return toString(::cos(toNumber(tok)));
}

std::string MathLib::tan(const std::string &tok)
{
    return toString(::tan(toNumber(tok)));
}


std::string MathLib::abs(const std::string &tok)
{
    return toString(::abs(toNumber(tok)));
}

bool MathLib::isGreater(const std::string &first, const std::string &second)
{
    return toNumber(first) > toNumber(second);
}

bool MathLib::isEqual(const std::string &first, const std::string &second)
{
    return toNumber(first) == toNumber(second);
}

bool MathLib::isLess(const std::string &first, const std::string &second)
{
    return toNumber(first) < toNumber(second);
}

int MathLib::compare(const std::string &first, const std::string &second)
{
    if (toNumber(first) < toNumber(second))
        return -1;
    if (toNumber(first) == toNumber(second))
        return 0;
    return 1;
}

