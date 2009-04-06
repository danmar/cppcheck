#include "mathLib.h"


#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>

double MathLib::ToNumber(const std::string &str)
{
	return atof(str.c_str());
}

std::string MathLib::ToString(double d)
{
	std::ostringstream result;
	result<<d;
	return result.str(); 
}

std::string MathLib::add(const std::string & first, const std::string & second)
{
	return ToString(ToNumber(first) + ToNumber(second));
}

std::string MathLib::substract(const std::string &first, const std::string &second)
{
	return ToString(ToNumber(first) - ToNumber(second));
}

std::string MathLib::divide(const std::string &first, const std::string &second)
{
	return ToString(ToNumber(first) / ToNumber(second));
}

std::string MathLib::multiply(const std::string &first, const std::string &second)
{
	return ToString(ToNumber(first) * ToNumber(second));
}


std::string MathLib::sin(const std::string &tok)
{
	return ToString(::sin(ToNumber(tok)));
}


std::string MathLib::cos(const std::string &tok)
{
	return ToString(::cos(ToNumber(tok)));
}

std::string MathLib::tan(const std::string &tok)
{
	return ToString(::tan(ToNumber(tok)));
}


std::string MathLib::abs(const std::string &tok)
{
	return ToString(::abs(ToNumber(tok)));
}

bool MathLib::IsGreater(const std::string &first, const std::string &second)
{
	return ToNumber(first) > ToNumber(second);
}

bool MathLib::IsEqual(const std::string &first, const std::string &second)
{
	return ToNumber(first) == ToNumber(second);
}

bool MathLib::IsLess(const std::string &first, const std::string &second)
{
	return ToNumber(first) < ToNumber(second);
}

int MathLib::compare(const std::string &first, const std::string &second)
{
	if(ToNumber(first) < ToNumber(second))
		return -1;
	if(ToNumber(first) == ToNumber(second))
		return 0;
	return 1;
}

