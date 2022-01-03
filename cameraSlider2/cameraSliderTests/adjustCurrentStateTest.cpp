#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include "adjustCurrentState.h"
#include <string>

using namespace std;

class AdjustReadingTestCase: public CppUnit::TestCase {
	CPPUNIT_TEST_SUITE(AdjustReadingTestCase);
	CPPUNIT_TEST(testLowerBoundary);
	CPPUNIT_TEST(testHigherBoundary);
	CPPUNIT_TEST_SUITE_END();

	adjustCurrentState *currentState = NULL;
	void			testLowerBoundary();
	void			testHigherBoundary();

public:
	void			setUp();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AdjustReadingTestCase, "AdjustReadingTestCase");

void AdjustReadingTestCase::setUp() {
	if (currentState != NULL) {
		delete currentState;
	}
}

// int adjustCurrentState(int reading, int currentState, int min, int max);
void AdjustReadingTestCase::testLowerBoundary() {
	currentState = new adjustCurrentState(99, 20, 1, 20);
	CPPUNIT_ASSERT_MESSAGE("Reading is " + to_string(currentState->getCurrent()),
			currentState->getCurrent() == 20);
	delete currentState;
	currentState = new adjustCurrentState(99, 99, 1, 20);
	CPPUNIT_ASSERT_MESSAGE("Reading is " + to_string(currentState->getCurrent()),
			currentState->getCurrent() == 20);
	delete currentState;
	currentState = new adjustCurrentState(99, 19, 1, 20);
	CPPUNIT_ASSERT_MESSAGE("Reading is " + to_string(currentState->getCurrent()),
			currentState->getCurrent() == 20);
	delete currentState;
	currentState = new adjustCurrentState(99, 18, 1, 20);
	CPPUNIT_ASSERT_MESSAGE("Reading is " + to_string(currentState->getCurrent()),
			currentState->getCurrent() == 20);
	delete currentState;
	currentState = new adjustCurrentState(99, 1, 1, 20);
	CPPUNIT_ASSERT_MESSAGE("Reading is " + to_string(currentState->getCurrent()),
			currentState->getCurrent() == 20);
	delete currentState;
	currentState = new adjustCurrentState(99, 2, 1, 20);
	CPPUNIT_ASSERT_MESSAGE("Reading is " + to_string(currentState->getCurrent()),
			currentState->getCurrent() == 20);
	delete currentState;

}

// int adjustCurrentState(int reading, int currentState, int min, int max);
void AdjustReadingTestCase::testHigherBoundary() {

}

CppUnit::Test *suite() {
  CppUnit::TestFactoryRegistry &registry =
					  CppUnit::TestFactoryRegistry::getRegistry();


  registry.registerFactory(
	  &CppUnit::TestFactoryRegistry::getRegistry( "AdjustReadingTestCase" ) );
  return registry.makeTest();
}



int main(int argc, char* argv[]) {
  // if command line contains "-selftest" then this is the post build check
  // => the output must be in the compiler error format.
  bool selfTest = (argc > 1)  &&
				  (std::string("-selftest") == argv[1]);


  CppUnit::TextUi::TestRunner runner;
  runner.addTest( suite() );   // Add the top suite to the test runner


  if (selfTest) {
	// Change the default outputter to a compiler error format outputter
	// The test runner owns the new outputter.
	runner.setOutputter( CppUnit::CompilerOutputter::defaultOutputter(
														&runner.result(),
														std::cerr ) );
  }


  // Run the test.
  bool wasSucessful = runner.run( "" );


  // Return error code 1 if any tests failed.
  return wasSucessful ? 0 : 1;
}
