#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HBridgeMotor.h"
#include "mockArduinoInclude.h"

class HBridgeMotorTestCase: public CppUnit::TestCase {
  CPPUNIT_TEST_SUITE(HBridgeMotorTestCase);
  CPPUNIT_TEST(testInit);
  CPPUNIT_TEST(testCommit);
  CPPUNIT_TEST(testReverse);
  CPPUNIT_TEST(testRepeatedCommand);
  CPPUNIT_TEST_SUITE_END();

	HBridgeMotor	*		motor;
	HBridgeMotorDirection	direction;
	void			testInit();
	void			testCommit();
	void			testReverse();
	void			testRepeatedCommand();

public:
	void			setUp();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(HBridgeMotorTestCase, "HBridgeMotorTestCase");

void HBridgeMotorTestCase::setUp() {
	initializeMockArduino();
	motor = new HBridgeMotor(1,2);
	motor->init();
}

void HBridgeMotorTestCase::testInit() {
	CPPUNIT_ASSERT_MESSAGE("port 1 is not output: " + to_string(pinModeData[1]), pinModeData[1] == OUTPUT);
	CPPUNIT_ASSERT_MESSAGE("port 2 is not output: " + to_string(pinModeData[2]), pinModeData[2] == OUTPUT);
}

void HBridgeMotorTestCase::testCommit() {
	direction = HBM_LEFT;
	motor->setSpeed(100);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[1]), analogPortData[1] == 100);
	CPPUNIT_ASSERT_MESSAGE("Port enabling graduality problem", testGraduality(analogPortWritings[1], 0, 1, 100));
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("No writes to other motor ports: ", testNoOther(1,2));
	clearPortWritings();
	direction = HBM_RIGHT;
	motor->setSpeed(255);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[2]), analogPortData[2] == 255);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("Port should go from 100 to 0", testGraduality(analogPortWritings[1], 0, 99, 0));
	CPPUNIT_ASSERT_MESSAGE("And after it is 0, it should go to 255", testGraduality(analogPortWritings[2], 99, 0, 255));
	CPPUNIT_ASSERT_MESSAGE("Both motor directions enabled at the same time", testNoParallel(analogPortWritings, 1, 2));
	CPPUNIT_ASSERT_MESSAGE("No writes to other motor ports: ", testNoOther(1,2));
	clearPortWritings();
	motor->setSpeed(0);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[2]), analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("Port should go from 254 to 0", testGraduality(analogPortWritings[2], 0, 254, 0));
	CPPUNIT_ASSERT_MESSAGE("Both motor directions enabled at the same time", testNoParallel(analogPortWritings, 1, 2));
	CPPUNIT_ASSERT_MESSAGE("No writes to other motor ports: ", testNoOther(1,2));
}

void HBridgeMotorTestCase::testReverse() {
	direction = HBM_RIGHT;
	motor->setSpeed(255);
	motor->setDirection(direction);
	motor->commit();
	motor->stop();
	CPPUNIT_ASSERT_MESSAGE("Motor failed to stop", analogPortData[1] == 0 && analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("Port should go from 254 to 0", testGraduality(analogPortWritings[2], 0, 254, 0));
	CPPUNIT_ASSERT_MESSAGE("Both motor directions enabled at the same time", testNoParallel(analogPortWritings, 1, 2));
	motor->reverse();
	CPPUNIT_ASSERT_MESSAGE("Motor not stopped after stop and reverse", analogPortData[1] == 0 && analogPortData[2] == 0);
	motor->setSpeed(255);
	motor->setDirection(direction);
	motor->commit();
	motor->reverse();
	direction = HBM_LEFT;
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[2]), analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[1]), analogPortData[1] == 255);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("No writes to other motor ports: ", testNoOther(1,2));
	CPPUNIT_ASSERT_MESSAGE("No writes to other motor ports: ", testNoOther(1,2));
	motor->reverse();
	direction = HBM_RIGHT;
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[1]), analogPortData[1] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[2]), analogPortData[2] == 255);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	motor->setSpeed(200);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[1]), analogPortData[1] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[2]), analogPortData[2] == 200);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	motor->setSpeed(250);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[1]), analogPortData[1] == 0);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[2]), analogPortData[2] == 250);
	motor->reverse();
	direction = HBM_LEFT;
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[1]), analogPortData[1] == 250);
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[2]), analogPortData[2] == 0);
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("No writes to other motor ports: ", testNoOther(1,2));
}

void HBridgeMotorTestCase::testRepeatedCommand() {
	direction = HBM_RIGHT;
	motor->setSpeed(200);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[2]), analogPortData[2] == 200);
	CPPUNIT_ASSERT_MESSAGE("Port enabling graduality problem", testGraduality(analogPortWritings[2], 0, 1, 200));
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("No writes to other motor ports: ", testNoOther(1,2));
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[1]), analogPortData[2] == 200);
	CPPUNIT_ASSERT_MESSAGE("Port continues flow without being resetted: " + to_string(analogPortWritings[2][199]),
			(analogPortWritings[2][199] == 200));
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("No writes to other motor ports: ", testNoOther(1,2));

	motor->setSpeed(0);
	motor->commit();
	clearPortWritings();
	direction = HBM_LEFT;
	motor->setSpeed(200);
	motor->setDirection(direction);
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[1]), analogPortData[1] == 200);
	CPPUNIT_ASSERT_MESSAGE("Port enabling graduality problem", testGraduality(analogPortWritings[1], 0, 1, 200));
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("No writes to other motor ports: ", testNoOther(1,2));
	motor->commit();
	CPPUNIT_ASSERT_MESSAGE("AnalogPort Contains: " + to_string(analogPortData[1]), analogPortData[1] == 200);
	CPPUNIT_ASSERT_MESSAGE("Port continues flow without being resetted: " + to_string(analogPortWritings[1][199]),
			(analogPortWritings[1][199] == 200));
	CPPUNIT_ASSERT_MESSAGE("Current direction is: " + to_string(motor->getDirection()) + " expected: " + to_string(direction), (int) motor->getDirection() == (int) direction);
	CPPUNIT_ASSERT_MESSAGE("No writes to other motor ports: ", testNoOther(1,2));
}

CppUnit::Test *suite() {
  CppUnit::TestFactoryRegistry &registry =
					  CppUnit::TestFactoryRegistry::getRegistry();


  registry.registerFactory(
	  &CppUnit::TestFactoryRegistry::getRegistry( "HBridgeMotorTestCase" ) );
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
