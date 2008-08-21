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

#ifndef MiniCppUnit_hxx
#define MiniCppUnit_hxx

/**
 * @mainpage
 * miniCppUnit 
 * (C) 2003-2006 Pau Arumi & David Garcia
 * 
 * @version 2.5 2006-03-14
 *   - MS Visual compatibility: SConstruct ccflags, usage example, #ifdefs
 * @version 2.4 2006-03-14
 *   - exit test case after first failure
 *   - double and float comparison with fuzzy equals (using scalable epsilon)
 *   - have into account not a numbers
 *   - new ASSERT_EQUALS_EPSILON macro
 *   - more colors, and disabled when comiled in MS Visual
 *   - removed catalan location.
 *   - UsageExample.cxx now uses all macros and features
 * @version 2.3 2006-02-13 added usage example and SConstruct
 * @version 2.2 2004-11-28 code in english and tests suites
 * @version 2.1 2004-11-04 char* especialization
 * @version 2.0 2004-10-26 TestsFactory
 * @version 1.0 2003-10-28 initial
 * 
 * Example of use:
 *
 * @code
 * #include "MiniCppUnit.hxx"
 * class MyTests : public TestFixture<MyTests>
 * {
 *  public:
 *  	TEST_FIXTURE( MyTests )
 *		{
 *			CAS_DE_TEST( testAddition );
 *			// etc
 *		}
 *		void testAddition()
 *		{ 
 *			ASSERT_EQUALS( 4, 1+1+2 );
 *		}  
 *		// etc
 * };
 *
 * REGISTER_FIXTURE( MyTests );
 * @endcode
 * @code
 * int main()
 * {
 *	return TestFixtureFactory::theInstance().runTests() ? 0 : -1;
 * }
 * @endcode
 * Good things: 
 *
 *   - it's a tiny framework made up of two or three src files. 
 *     => no need to install as a library
 *   - object oriented and makes use of several GoF patterns 
 *   - very simple usage. Just needs to learn very few C macros
 *   - string asserts are simpler to use than cppunit
 *   - string asserts are enhanced with coloured diffs
 *   - concrete test classes are totally decoupled via static factory
 *     => no src file have to include them all.
 *   - it have test suite hierarchies 
 *   - compatible with non-standard compliant VisualC6 
 *     (though not necessary good ;)
 */

#include <iostream>
#include <string>
#include <sstream>
#include <list>

/**
 * A singleton class. 
 * Receives tests results and stores messages to the test log
 * for later listing.
 * It's a singleton for an easy global access from the 'Asserts'
 * methods but it is probably asking for a refactoring in order to limit
 * access only to TestFixtures
 */
class TestsListener
{
public:
	/** accessor to the global (static) singleton instance */
	static TestsListener& theInstance();
	std::stringstream& errorsLog();
	std::string logString();
	void currentTestName( std::string& name);
	static void testHasRun();
	static void testHasFailed();
	static void testHasThrown();
	/** the human readable summary of run tests*/
	std::string summary();
	/** returns wheather all run tests have passed */
	static bool allTestsPassed();
	
private:
	static const char* errmsgTag_nameOfTest() { return "Test failed: "; }
	
	/** constructor private: force the singleton to be wellbehaved ! */
	TestsListener() : _currentTestName(0)
	{
		_executed=_failed=_exceptions=0;
	}
	
	std::string* _currentTestName;
	std::stringstream _log;
	unsigned _executed;
	unsigned _failed;
	unsigned _exceptions;
};

class TestFailedException
{
};

/**
 * Abstract class with interface that allows run a test. That is runTest
 * and name. It is implemented by TestFixture and TestCase
 *
 * It does the 'Component' role in the 'Composite' patten
 **/
class Test
{
public:
	virtual ~Test(){}
	/** run the test: exercice the code and check results*/
	virtual void runTest() = 0;
	/** the test human-readable name */
	virtual std::string name() const = 0;
};


/**
 * This class is just a placeholder for all assert functions --as static methods.
 * It is meant for being used just by the assert macros
 */
class Assert
{
	static const char * errmsgTag_testFailedIn() { return "Test failed in "; }
	static const char * errmsgTag_inLine() { return ", line: "; };
	static const char * errmsgTag_failedExpression() { return "Failed expression: "; } 
	static const char * errmsgTag_expected() { return "Expected: "; } 
	static const char * errmsgTag_butWas() { return "But was: "; } 

public:
#ifdef _MSC_VER
	static const char * blue() { return ""; }
	static const char * green() { return ""; }
	static const char * red() { return ""; }
	static const char * normal() { return ""; }
	static const char * bold() { return ""; }
	static const char * yellow() { return ""; }
#else
	static const char * blue() { return "\033[36;1m"; }
	static const char * green() { return "\033[32;1m"; }
	static const char * red() { return "\033[31;1m"; }
	static const char * normal() { return "\033[0m"; }
	static const char * bold() { return "\033[" "1m"; }
	static const char * yellow() { return "\033[93;1m"; }
#endif
	template<typename AType>
	static void assertEquals( const AType& expected, const AType& result,
		const char* file="", int linia=0 )
	{
		if(expected != result)
		{
			TestsListener::theInstance().errorsLog() 
				<< file << ", linia: " << linia << "\n"
				<< errmsgTag_expected() << " " << expected << " "
				<< errmsgTag_butWas() << " " << result << "\n";
			TestsListener::theInstance().testHasFailed();
		}
	}

	static void assertTrue(char* strExpression, bool expression,
			const char* file="", int linia=0);

	static void assertTrueMissatge(char* strExpression, bool expression, 
			const char* missatge, const char* file="", int linia=0);

	static void assertEquals( const char * expected, const char * result,
		const char* file="", int linia=0 );
	
	static void assertEquals( const bool& expected, const bool& result,
		const char* file="", int linia=0 );
	
	static void assertEquals( const double& expected, const double& result,
		const char* file="", int linia=0 );

	static void assertEquals( const float& expected, const float& result,
		const char* file="", int linia=0 );
	
	static void assertEquals( const long double& expected, const long double& result,
		const char* file="", int linia=0 );
	
	static void assertEqualsEpsilon( const double& expected, const double& result, const double& epsilon,
		const char* file="", int linia=0 );

	static int notEqualIndex( const std::string & one, const std::string & other );

	/**
	 * we overload the assert with string doing colored diffs
	 *
	 * MS Visual6 doesn't allow string by reference :-( 
	 */
	static void assertEquals( const std::string expected, const std::string result,
		const char* file="", int linia=0 );
	
	static void fail(const char* motiu, const char* file="", int linia=0);


};

/**
 * A TestFixture is a class that contain TestCases --which corresponds to 
 * ConcreteTestFixture methods-- common objects uder tests, and setUp and
 * tearDown methods which are automatically executed before and after each
 * test case.
 *
 * Is the base class of ConcreteFixtures implemented by the framework user
 *
 * It does the 'Composite' role in the 'Composite' GoF pattern.
 * Its composite children are TestCases, which wrapps the test methods.
 *
 * It is a template class parametrized by ConcreteTestFixture so that it can 
 * instantiate TestCase objects templatized with this same parameter: it needs the 
 * concrete class type for calling its non-static methods.
 */
template <typename ConcreteTestFixture>
class TestFixture : public Test
{
protected:

	typedef ConcreteTestFixture ConcreteFixture;
	typedef void(ConcreteTestFixture::*TestCaseMethod)();

	/**
	 * Wrapper for the test methods of concrete TestFixtures.
	 *
	 * Makes the 'Leave' role in the 'Composite' GoF pattern because can't be
	 * be a composition of other tests.
	 * 
	 * It's also a case of 'Command' pattern because it encapsules in an object 
	 * certain functionality whose execution depends on some deferred entity.
	 */
	class TestCase : public Test
	{
	public:
		TestCase(ConcreteFixture* parent, TestCaseMethod method, const std::string & name) : 
		  _parent(parent),
		  _testCaseMethod(method),
		  _name(name)
		{
		}
		/** calls TestFixture method.  setUp and tearDown methods are called by
		 * its parent TestFixture (in its runTest method).
		 * it is robust to unexpected exceptions (throw) */
		void runTest()
		{
			TestsListener::theInstance().testHasRun();
			TestsListener::theInstance().currentTestName(_name);
			try
			{
				(_parent->*_testCaseMethod)();
			}
			catch( std::exception& error )
			{
				TestsListener::theInstance().testHasThrown();
				TestsListener::theInstance().errorsLog() 
					<< "std::exception catched by MiniCppUnit: \n"
					<< "what() : " 
					<< Assert::yellow() << error.what() 
					<< Assert::normal() << "\n";
			}
			catch ( TestFailedException& failure) //just for skiping current test case
			{
			}
			catch(...)
			{
				TestsListener::theInstance().testHasThrown();
				TestsListener::theInstance().errorsLog() 
					<< "non standard exception catched by MiniCppUnit.\n";
			}
		}

		/** the TestFixture method hame */
		std::string name() const
		{
			return _name;
		}

	private:
		ConcreteFixture* _parent;
		TestCaseMethod _testCaseMethod;
		std::string _name;
	};
    //------------- end of class TestCase ----------------------------

private:
	
	typedef std::list<Test*> TestCases;
	TestCases _testCases;
	std::string _name;

	void testsList() const
	{
		std::cout << "\n+ " << name() << "\n";
		for( TestCases::const_iterator it=_testCases.begin(); 
			it!=_testCases.end(); it++ )
			std::cout << "  - "<< (*it)->name() << "\n";
	}
	

public:
	virtual void setUp() {}
	virtual void tearDown() {}

	std::string name() const 
	{
		return _name;
	};

	TestFixture(const std::string& name="A text fixture") : _name(name)
	{
	}

	void afegeixCasDeTest(ConcreteFixture* parent, TestCaseMethod method, const char* name)
	{
		TestCase* casDeTest = new TestCase(parent, method, _name + "::" + name);
		_testCases.push_back( casDeTest );
	}
	/** calls each test after setUp and tearDown TestFixture methods */
	void runTest()
	{
		testsList();
		TestCases::iterator it;
		for( it=_testCases.begin(); it!=_testCases.end(); it++)
		{
			setUp();
			(*it)->runTest();
			tearDown();
		}
	}
	/** TestCase that wrapps TestFixture methods are dynamically created and owned by 
	 * the TestFixture. So here we clean it up*/
	~TestFixture()
	{	
		TestCases::iterator it;
		for( it =_testCases.begin(); it!=_testCases.end(); it++)
			delete (*it);
	}
};


/**
 * This class is aimed to hold a creator method for each concrete TestFixture
 */
class TestFixtureFactory
{
private:
	/** Well behaved singleton: 
	 *  Don't allow instantiation apart from theInstance(), so private ctr.*/
	TestFixtureFactory()
	{
	}
	typedef Test* (*FixtureCreator)();
	std::list<FixtureCreator> _creators;
public:
	/** Accessor to the (static) singleton instance */
	static TestFixtureFactory& theInstance()
	{
		static TestFixtureFactory theFactory;
		return theFactory;
	}
	bool runTests()
	{
		std::list<FixtureCreator>::iterator it;
		for(it=_creators.begin(); it!=_creators.end(); it++)
		{	
			FixtureCreator creator = *it;
			Test* test = creator();
			test->runTest();
			delete test;
		}
		std::string errors =  TestsListener::theInstance().logString();
		if (errors!="") std::cout << "\n\nError Details:\n" << errors;
		std::cout << TestsListener::theInstance().summary();

		return TestsListener::theInstance().allTestsPassed();	
	}
	void addFixtureCreator(FixtureCreator creator)
	{
		_creators.push_back( creator );
	}
	
};

/** 
 * Macro a usar despr�s de cada classe de test
 */
#define REGISTER_FIXTURE( ConcreteTestFixture ) \
\
Test* Creador##ConcreteTestFixture() { return new ConcreteTestFixture; } \
\
class Registrador##ConcreteTestFixture \
{ \
public: \
	Registrador##ConcreteTestFixture() \
	{ \
		TestFixtureFactory::theInstance().addFixtureCreator( \
				Creador##ConcreteTestFixture); \
	} \
}; \
static Registrador##ConcreteTestFixture estatic##ConcreteTestFixture;


/**
 * Assert macros to use in test methods. An assert is a test condition
 * we want to check.
 */
#define ASSERT_EQUALS( expected, result) \
	Assert::assertEquals( expected, result, __FILE__, __LINE__ );

#define ASSERT_EQUALS_EPSILON( expected, result, epsilon) \
	Assert::assertEqualsEpsilon( expected, result, epsilon, __FILE__, __LINE__ );

#define ASSERT( exp ) \
	Assert::assertTrue(#exp, exp, __FILE__, __LINE__);

#define ASSERT_MESSAGE( exp, message ) \
	Assert::assertTrueMissatge(#exp, exp, message, __FILE__, __LINE__);

#define FAIL( why ) \
	Assert::fail(#why, __FILE__, __LINE__);

/**
 * Macros that allows to write the  constructor of the concrete TestFixture.
 * What the constructor does is agregate a wrapper for each test case (method)
 * As easy to write as this:
 *
 * @code
 * class MyTests : public TestFixture<MyTests>
 * {
 *  public:
 *  	TEST_FIXTURE( MyTests )
 *	{
 *		TEST_CASE( test );
 *		// etc
 *	}
 *	void test()
 *	{
 *		ASSERT_EQUALS( 4, 1+1+2 );
 *	}
 * @endcode
 */

#define TEST_FIXTURE( ConcreteFixture ) \
	ConcreteFixture() : TestFixture<ConcreteFixture>( #ConcreteFixture )

#define TEST_CASE( methodName ) \
	afegeixCasDeTest( this, &ConcreteFixture::methodName, #methodName );




			     
#endif  // MiniCppUnit_hxx
