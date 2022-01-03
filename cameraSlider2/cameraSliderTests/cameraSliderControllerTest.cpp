#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include "cameraSliderController.h"
#include <string>
#include <assert.h>

using namespace std;

class CameraSliderControllerTestCase: public CppUnit::TestCase {
  CPPUNIT_TEST_SUITE(CameraSliderControllerTestCase);
  CPPUNIT_TEST(testCameraSlider);
  CPPUNIT_TEST(testRepeat);
  CPPUNIT_TEST(testFinish);
  CPPUNIT_TEST_SUITE_END();

  	cameraSliderController	* controller;
	double			m_value1;
	double			m_value2;
	void			testCameraSlider();
	void			testRepeat();
	void			testFinish();

public:
	void			setUp();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(CameraSliderControllerTestCase, "CameraSliderControllerTestCase");

void CameraSliderControllerTestCase::setUp() {
	controller = new cameraSliderController();
}

void CameraSliderControllerTestCase::testCameraSlider() {
	controller->setUserInput(UI_RIGHT);
	CPPUNIT_ASSERT_MESSAGE("enable right, see if goes right: " + to_string(controller->getDirection()),
			controller->getDirection() == HBM_RIGHT);
	controller->setUserInput(UI_LEFT);
	CPPUNIT_ASSERT_MESSAGE("enable left, see if goes left: " + to_string(controller->getDirection()),
			controller->getDirection() == HBM_LEFT);
	controller->sliderRightReached(1);
	CPPUNIT_ASSERT_MESSAGE("right sensor reached, still goes left: " + to_string(controller->getDirection()),
			controller->getDirection() == HBM_LEFT);
	controller->sliderRightReached(0);
	controller->sliderLeftReached(1);
	CPPUNIT_ASSERT_MESSAGE("left sensor reached, stopped: " + to_string(controller->getDirection()),
			controller->getDirection() == HBM_STOP);
	controller->setUserInput(UI_RIGHT);
	CPPUNIT_ASSERT_MESSAGE("right requested, going right: " + to_string(controller->getDirection()),
			controller->getDirection() == HBM_RIGHT);
	controller->sliderRightReached(1);
	controller->sliderLeftReached(1);
	CPPUNIT_ASSERT_MESSAGE("both sensors enabled = stop: " + to_string(controller->getDirection()),
			controller->getDirection() == HBM_STOP);
	controller->sliderRightReached(0);
	controller->sliderLeftReached(0);
	controller->setUserInput(UI_RIGHT);
	controller->setUserInput(UI_STOP);
	CPPUNIT_ASSERT_MESSAGE("go right, then ask for stop = stop: " + to_string(controller->getDirection()),
			controller->getDirection() == HBM_STOP);
	controller->setUserInput(UI_LEFT);
	controller->setUserInput(UI_STOP);
	CPPUNIT_ASSERT_MESSAGE("go left, then ask for stop = stop: " + to_string(controller->getDirection()),
			controller->getDirection() == HBM_STOP);
}

void CameraSliderControllerTestCase::testRepeat() {
	controller->setRepeatEnable(1);
	controller->setUserInput(UI_RIGHT);
	controller->sliderRightReached(1);
	CPPUNIT_ASSERT_MESSAGE("enable repeat, go right, reach right sensor, check if going left: " + 
			to_string(controller->getDirection()), controller->getDirection() == HBM_LEFT);
	controller->sliderRightReached(0);
	controller->setUserInput(UI_RIGHT);
	CPPUNIT_ASSERT_MESSAGE("command to go right, see if going left: " + 
			to_string(controller->getDirection()), controller->getDirection() == HBM_LEFT);
	controller->sliderLeftReached(1);
	CPPUNIT_ASSERT_MESSAGE("reach left sensor, see of going right: " + 
			to_string(controller->getDirection()), controller->getDirection() == HBM_RIGHT);
	controller->setUserInput(UI_RIGHT);
	CPPUNIT_ASSERT_MESSAGE("command to go right, see if still going right: " + 
			to_string(controller->getDirection()), controller->getDirection() == HBM_RIGHT);
	controller->sliderLeftReached(0);
	controller->sliderRightReached(1);
	CPPUNIT_ASSERT_MESSAGE("reach right sensor, see if going left: " + 
			to_string(controller->getDirection()), controller->getDirection() == HBM_LEFT);
	CPPUNIT_ASSERT_MESSAGE("and still going left: " +
			to_string(controller->getDirection()), controller->getDirection() == HBM_LEFT);
	CPPUNIT_ASSERT_MESSAGE("and still going left: " +
			to_string(controller->getDirection()), controller->getDirection() == HBM_LEFT);
	controller->setUserInput(UI_RIGHT);
	controller->sliderRightReached(1);
	CPPUNIT_ASSERT_MESSAGE("still request right, reach right sensor, see if going left: " + 
			to_string(controller->getDirection()), controller->getDirection() == HBM_LEFT);
	controller->sliderRightReached(0);
	controller->setUserInput(UI_STOP);
	controller->setUserInput(UI_RIGHT);
	// controller->printState(); // useful for state debugging
	CPPUNIT_ASSERT_MESSAGE("user request to stop, then right, see if going right: " + 
			to_string(controller->getDirection()), controller->getDirection() == HBM_RIGHT);
	CPPUNIT_ASSERT_MESSAGE("and again: " +
			to_string(controller->getDirection()), controller->getDirection() == HBM_RIGHT);
	CPPUNIT_ASSERT_MESSAGE("and again: " +
			to_string(controller->getDirection()), controller->getDirection() == HBM_RIGHT);

}

void CameraSliderControllerTestCase::testFinish() {
	controller->setRepeatEnable(1);
	controller->setUserInput(UI_RIGHT);
	controller->sliderRightReached(1);
	CPPUNIT_ASSERT_MESSAGE("enable repeat, go right, reach right sensor, check if going left: " + 
			to_string(controller->getDirection()), controller->getDirection() == HBM_LEFT);
	controller->sliderRightReached(0);
	controller->setUserInput(UI_RIGHT);
	CPPUNIT_ASSERT_MESSAGE("command to go right, see if going left: " + 
			to_string(controller->getDirection()), controller->getDirection() == HBM_LEFT);
	controller->finish();
	controller->setUserInput(UI_RIGHT);
	CPPUNIT_ASSERT_MESSAGE("do finish and check if going right again really goes right: " +
			to_string(controller->getDirection()), controller->getDirection() == HBM_RIGHT);
}

CppUnit::Test *suite() {
  CppUnit::TestFactoryRegistry &registry =
					  CppUnit::TestFactoryRegistry::getRegistry();


  registry.registerFactory(
	  &CppUnit::TestFactoryRegistry::getRegistry( "CameraSliderControllerTestCase" ) );
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
