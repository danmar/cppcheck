/*
 * Copyright (c) 2003-2004  Pau Arum� & David Garc�a
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "MiniCppUnit.h"

#include <cmath>

#ifdef _MSC_VER
#include <float.h>
namespace std
{
  template <typename T>
  inline bool isnan(T x) { 
		return _isnan(x) != 0; 
	}
	template <typename T>
	inline bool isinf(T x) { 
		return _finite(x) == 0; 
	}
}
#endif

TestsListener& TestsListener::theInstance()
{
	static TestsListener instancia;
	return instancia;
}

std::stringstream& TestsListener::errorsLog()
{
	if (_currentTestName)
		_log << "\n" << errmsgTag_nameOfTest() << (*_currentTestName) << "\n";
	return _log;
}

std::string TestsListener::logString()
{
	std::string aRetornar = _log.str();
	_log.str("");
	return aRetornar;
}
void TestsListener::currentTestName( std::string& name)
{
	_currentTestName = &name;
}
void TestsListener::testHasRun()
{
	std::cout << ".";
	theInstance()._executed++;
}
void TestsListener::testHasFailed()
{
	std::cout << "F";
	theInstance()._failed++;
	throw TestFailedException();
}
void TestsListener::testHasThrown()
{
	std::cout << "E";
	theInstance()._exceptions++;
}
std::string TestsListener::summary()
{
	std::ostringstream os;
	os	<< "\nSummary:\n"
		<< Assert::bold() << "\tExecuted Tests:         " 
		<< _executed << Assert::normal() << std::endl
		<< Assert::green() << "\tPassed Tests:           " 
		<< (_executed-_failed-_exceptions) 
		<< Assert::normal() << std::endl;
	if (_failed > 0)
	{
		os 	<< Assert::red() << "\tFailed Tests:           " 
			<< _failed << Assert::normal() << std::endl;
	}
	if (_exceptions > 0)
	{
		os 	<< Assert::yellow() << "\tUnexpected exceptions:  " 
			<< _exceptions << Assert::normal() << std::endl;
	}
	os << std::endl;
	return os.str();
}
bool TestsListener::allTestsPassed()
{
	return !theInstance()._exceptions && !theInstance()._failed;
}



void Assert::assertTrue(char* strExpression, bool expression,
		const char* file, int linia)
{
	if (!expression)
	{
		TestsListener::theInstance().errorsLog() << "\n"
			<< errmsgTag_testFailedIn() << file 
			<< errmsgTag_inLine() << linia << "\n" 
			<< errmsgTag_failedExpression() 
			<< bold() << strExpression << normal() << "\n";
		TestsListener::theInstance().testHasFailed();
	}
}

void Assert::assertTrueMissatge(char* strExpression, bool expression, 
		const char* missatge, const char* file, int linia)
{
	if (!expression)
	{
		TestsListener::theInstance().errorsLog() << "\n"
			<< errmsgTag_testFailedIn() << file
			<< errmsgTag_inLine() << linia << "\n" 
			<< errmsgTag_failedExpression() 
			<< bold() << strExpression << "\n"
			<< missatge<< normal() << "\n";
		TestsListener::theInstance().testHasFailed();
	}
}



void Assert::assertEquals( const char * expected, const char * result,
	const char* file, int linia )
{
	assertEquals(std::string(expected), std::string(result),
		file, linia);

}
void Assert::assertEquals( const bool& expected, const bool& result,
	const char* file, int linia )
{
	assertEquals(
		(expected?"true":"false"), 
		(result?"true":"false"),
		file, linia);
}

// floating point numbers comparisons taken
// from c/c++ users journal. dec 04 pag 10
bool isNaN(double x)
{
	bool b1 = (x < 0.0);
	bool b2 = (x >= 0.0);
	return !(b1 || b2);
}

double scaledEpsilon(const double& expected, const double& fuzzyEpsilon )
{ 
	const double aa = fabs(expected)+1;
	return (std::isinf(aa))? fuzzyEpsilon: fuzzyEpsilon * aa;
}
bool fuzzyEquals(double expected, double result, double fuzzyEpsilon)
{
	return (expected==result) || ( fabs(expected-result) <= scaledEpsilon(expected, fuzzyEpsilon) );
}
void Assert::assertEquals( const double& expected, const double& result,
		const char* file, int linia )
{	
	const double fuzzyEpsilon = 0.000001;
	assertEqualsEpsilon( expected, result, fuzzyEpsilon, file, linia );
}

void Assert::assertEquals( const float& expected, const float& result,
		const char* file, int linia )
{
	assertEquals((double)expected, (double)result, file, linia);
}
void Assert::assertEquals( const long double& expected, const long double& result,
		const char* file, int linia )
{
	assertEquals((double)expected, (double)result, file, linia);
}
void Assert::assertEqualsEpsilon( const double& expected, const double& result, const double& epsilon,
		const char* file, int linia )
{
	if (isNaN(expected) && isNaN(result) ) return;
	if (!isNaN(expected) && !isNaN(result) && fuzzyEquals(expected, result, epsilon) ) return;

	TestsListener::theInstance().errorsLog() 
			<< errmsgTag_testFailedIn() << file
			<< errmsgTag_inLine() << linia << "\n" 
			<< errmsgTag_expected()
			<< bold() << expected << normal() << " "
			<< errmsgTag_butWas() 
			<< bold() << result << normal() << "\n";
	TestsListener::theInstance().testHasFailed();
}

int Assert::notEqualIndex( const std::string & one, const std::string & other )
{
	int end = std::min(one.length(), other.length());
	for ( int index = 0; index < end; index++ )
		if (one[index] != other[index] )
			return index;
	return end;
}


/**
 * we overload the assert with string doing colored diffs
 *
 * MS Visual6 doesn't allow string by reference :-( 
 */
void Assert::assertEquals( const std::string expected, const std::string result,
	const char* file, int linia )
{
	if(expected == result)
		return;
	
	int indexDiferent = notEqualIndex(expected, result);
	TestsListener::theInstance().errorsLog()
		<< file << ", line: " << linia << "\n"
		<< errmsgTag_expected() << "\n" << blue() 
		<< expected.substr(0,indexDiferent)
		<< green() << expected.substr(indexDiferent) 
		<< normal() << "\n"
		<< errmsgTag_butWas() << blue() << "\n" 
		<< result.substr(0,indexDiferent)
		<< red() << result.substr(indexDiferent) 
		<< normal() << std::endl;

	TestsListener::theInstance().testHasFailed();
}
void Assert::fail(const char* motiu, const char* file, int linia)
{
	TestsListener::theInstance().errorsLog() <<
		file << errmsgTag_inLine() << linia << "\n" <<
		"Reason: " << motiu << "\n";

	TestsListener::theInstance().testHasFailed();
}


