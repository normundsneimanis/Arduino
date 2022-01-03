#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include "DCMotor.h"
#include "mockArduinoInclude.h"

class DCMotorTestCase: public CppUnit::TestCase {
	CPPUNIT_TEST_SUITE(DCMotorTestCase);
	CPPUNIT_TEST(testInit);
	CPPUNIT_TEST(testCommit);
	CPPUNIT_TEST(testSmooth);
	CPPUNIT_TEST_SUITE_END();

	DCMotor	*		motor;
	void			testInit();
	void			testCommit();
	void			testSmooth();

public:
	void			setUp();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(DCMotorTestCase, "DCMotorTestCase");

void DCMotorTestCase::setUp() {
	initializeMockArduino();
	motor = new DCMotor(5);
	motor->init();
}

void DCMotorTestCase::testInit() {
	CPPUNIT_ASSERT_MESSAGE("port 5 is not output: " + to_string(pinModeData[5]), pinModeData[5] == OUTPUT);
}

void DCMotorTestCase::testCommit() {
	motor->setSpeed(100);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[5]), analogPortData[5] == 100);
	CPPUNIT_ASSERT_MESSAGE("Port enabling graduality problem", testGraduality(analogPortWritings[5], 0, 1, 100));
	CPPUNIT_ASSERT_MESSAGE("No writes to other except motor ports: ", testNoOther(5));
	clearPortWritings();
	motor->setSpeed(255);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[5]), analogPortData[5] == 255);
	CPPUNIT_ASSERT_MESSAGE("Port should go from 100 to 255", testGraduality(analogPortWritings[5], 0, 101, 255));
	CPPUNIT_ASSERT_MESSAGE("No writes to other except motor ports: ", testNoOther(5));
	clearPortWritings();
	motor->setSpeed(0);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[5]), analogPortData[5] == 0);
	CPPUNIT_ASSERT_MESSAGE("Port should go from 254 to 0", testGraduality(analogPortWritings[1], 0, 254, 0));
	CPPUNIT_ASSERT_MESSAGE("No writes to other except motor ports: ", testNoOther(5));
}

void DCMotorTestCase::testSmooth() {
	motor->setSpeed(200);
	motor->commit();
	motor->smoothStop();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[5]), analogPortData[5] == 0);
	CPPUNIT_ASSERT_MESSAGE("No writes to other except motor ports: ", testNoOther(5));
	clearPortWritings();
	motor->setSpeed(150);
	motor->smoothStart();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[5]), analogPortData[5] == 150);
	clearPortWritings();
	motor->setSpeed(100);
	motor->adjustSpeed();
	CPPUNIT_ASSERT_MESSAGE("Port should go from 149 to 100", testGraduality(analogPortWritings[5], 0, 149, 100));
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[5]), analogPortData[5] == 100);
	CPPUNIT_ASSERT_MESSAGE("No writes to other except motor ports: ", testNoOther(5));
	clearPortWritings();
	motor->setSpeed(200);
	motor->adjustSpeed();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[5]), analogPortData[5] == 200);
	CPPUNIT_ASSERT_MESSAGE("No writes to other except motor ports: ", testNoOther(5));
}

CppUnit::Test *suite() {
  CppUnit::TestFactoryRegistry &registry =
					  CppUnit::TestFactoryRegistry::getRegistry();


  registry.registerFactory(
	  &CppUnit::TestFactoryRegistry::getRegistry( "DCMotorTestCase" ) );
  return registry.makeTest();
}



int main(int argc, char* argv[]) {
  // if command line contains "-selftest" then this is the post build check
  // => the output must be in the compiler error format.
  bool selfTest = (argc > 1)  &&
				  (string("-selftest") == argv[1]);


  CppUnit::TextUi::TestRunner runner;
  runner.addTest( suite() );   // Add the top suite to the test runner


  if ( selfTest ) {
	// Change the default outputter to a compiler error format outputter
	// The test runner owns the new outputter.
	runner.setOutputter( CppUnit::CompilerOutputter::defaultOutputter(
														&runner.result(),
														cerr ) );
  }


  // Run the test.
  bool wasSucessful = runner.run( "" );


  // Return error code 1 if any tests failed.
  return wasSucessful ? 0 : 1;
}
