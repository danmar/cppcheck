#include "token.h"
class MathLib
{
private:
	static double ToNumber(const std::string & str);
	static std::string ToString(double d);
public:
	static std::string add(const std::string & first, const std::string & second);
	static std::string substract(const std::string & first, const std::string & second);
	static std::string multiply(const std::string & first, const std::string & second);
	static std::string divide(const std::string & first, const std::string & second);
	static std::string sin(const std::string & tok);
	static std::string cos(const std::string & tok);
	static std::string tan(const std::string & tok);
	static std::string abs(const std::string & tok);
	static bool IsGreater (const std::string & first, const std::string & second);
	static bool IsEqual (const std::string & first, const std::string & second);
	static bool IsLess (const std::string & first, const std::string & second);
	static int compare(const std::string & first, const std::string & second);
};