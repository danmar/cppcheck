#ifndef mathlibH
#define mathlibH

#include "token.h"

class MathLib
{
private:
    static double toNumber(const std::string & str);
    static std::string toString(double d);
public:
    static std::string add(const std::string & first, const std::string & second);
    static std::string subtract(const std::string & first, const std::string & second);
    static std::string multiply(const std::string & first, const std::string & second);
    static std::string divide(const std::string & first, const std::string & second);
    static std::string sin(const std::string & tok);
    static std::string cos(const std::string & tok);
    static std::string tan(const std::string & tok);
    static std::string abs(const std::string & tok);
    static bool isGreater(const std::string & first, const std::string & second);
    static bool isEqual(const std::string & first, const std::string & second);
    static bool isLess(const std::string & first, const std::string & second);
    static int compare(const std::string & first, const std::string & second);
};

#endif
