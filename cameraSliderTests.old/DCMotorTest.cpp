#define TEST 1
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
  CPPUNIT_TEST(testReverse);
  CPPUNIT_TEST(testSmooth);
  CPPUNIT_TEST_SUITE_END();

	DCMotor	*		motor;
	DCMotorDirection	direction;
	double			m_value1;
	double			m_value2;
	void			testInit();
	void			testCommit();
	void			testReverse();
	void			testSmooth();

public:
	void			setUp();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(DCMotorTestCase, "DCMotorTestCase");

void DCMotorTestCase::setUp() {
	initializeMockArduino();
	motor = new DCMotor(1,2);
}

void DCMotorTestCase::testInit() {
	CPPUNIT_ASSERT_MESSAGE("port 1 is not output: " + std::to_string(pinModeData[1]), pinModeData[1] == OUTPUT);
	CPPUNIT_ASSERT_MESSAGE("port 2 is not output: " + std::to_string(pinModeData[2]), pinModeData[2] == OUTPUT);
}

void DCMotorTestCase::testCommit() {
	clearPortWritings();
	direction = LEFT;
	motor->setSpeed(100);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 100);
	CPPUNIT_ASSERT_MESSAGE("Port enabling graduality problem", testGraduality(analogPortWritings, 0, 0, 100));
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + std::to_string(motor->getDirection()) + " expected: " + std::to_string(direction), (int) motor->getDirection() == (int) direction);
	clearPortWritings();
	direction = RIGHT;
	motor->setSpeed(255);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 255);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + std::to_string(motor->getDirection()) + " expected: " + std::to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("Port should go from 100 to 0", testGraduality(analogPortWritings, 0, 99, 0));
	CPPUNIT_ASSERT_MESSAGE("And after it is 0, it should go to 255", testGraduality(analogPortWritings, 99, 0, 255));
	clearPortWritings();
	motor->setSpeed(0);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 0);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + std::to_string(motor->getDirection()) + " expected: " + std::to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("Port should go from 254 to 0", testGraduality(analogPortWritings, 0, 254, 0));
}

void DCMotorTestCase::testReverse() {
	direction = RIGHT;
	motor->setSpeed(255);
	motor->setDirection(direction);
	motor->commit();
	motor->stop();
	CPPUNIT_ASSERT_MESSAGE("Motor failed to stop", analogPortData[1] == 0 && analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("Port should go from 254 to 0", testGraduality(analogPortWritings, 0, 254, 0));
	motor->reverse();
	CPPUNIT_ASSERT_MESSAGE("Motor not stopped after stop and reverse", analogPortData[1] == 0 && analogPortData[2] == 0);
	motor->setSpeed(255);
	motor->setDirection(direction);
	motor->commit();
	motor->reverse();
	direction = LEFT;
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 255);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + std::to_string(motor->getDirection()) + " expected: " + std::to_string(direction), (int) motor->getDirection() == (int) direction);
	motor->reverse();
	direction = RIGHT;
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 255);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + std::to_string(motor->getDirection()) + " expected: " + std::to_string(direction), (int) motor->getDirection() == (int) direction);
	motor->setSpeed(200);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 200);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + std::to_string(motor->getDirection()) + " expected: " + std::to_string(direction), (int) motor->getDirection() == (int) direction);
	motor->setSpeed(250);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 250);
	motor->reverse();
	direction = LEFT;
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 250);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 0);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + std::to_string(motor->getDirection()) + " expected: " + std::to_string(direction), (int) motor->getDirection() == (int) direction);
}

void DCMotorTestCase::testSmooth() {
	direction = RIGHT;
	motor->setSpeed(200);
	motor->setDirection(direction);
	motor->commit();
	motor->smoothStop();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 0);
	clearPortWritings();
	motor->setSpeed(150);
	motor->smoothStart();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 150);
	motor->setSpeed(100);
	motor->adjustSpeed();
	CPPUNIT_ASSERT_MESSAGE("Port should go from 149 to 100", testGraduality(analogPortWritings, 0, 149, 100));
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 100);
	direction = LEFT;
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 100);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 0);
	motor->setSpeed(200);
	motor->adjustSpeed();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 200);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 0);
	direction = RIGHT;
	motor->setDirection(direction);
	motor->adjustSpeed();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[2]), analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + std::to_string(analogPortData[1]), analogPortData[1] == 200);
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
				  (std::string("-selftest") == argv[1]);


  CppUnit::TextUi::TestRunner runner;
  runner.addTest( suite() );   // Add the top suite to the test runner


  if ( selfTest ) {
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
