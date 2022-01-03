test: adjustReadingStateTest cameraSliderControllerTest DCMotorTest HBridgeMotorTest

DCMotorTest: cameraSliderTests/mockArduino.cpp cameraSlider/DCMotor.cpp cameraSliderTests/DCMotorTest.cpp
	g++ -DTEST -g -std=gnu++11 -Wall -IcameraSlider -IcameraSliderTests cameraSliderTests/mockArduino.cpp cameraSlider/DCMotor.cpp cameraSliderTests/DCMotorTest.cpp -lcppunit -o DCMotorTest
	
HBridgeMotorTest: cameraSlider/DCMotor.cpp cameraSliderTests/mockArduino.cpp cameraSlider/HBridgeMotor.cpp cameraSliderTests/HBridgeMotorTest.cpp 
	g++ -DTEST -g -std=gnu++11 -Wall -IcameraSlider -IcameraSliderTests cameraSlider/DCMotor.cpp cameraSliderTests/mockArduino.cpp cameraSlider/HBridgeMotor.cpp cameraSliderTests/HBridgeMotorTest.cpp -lcppunit -o HBridgeMotorTest

adjustReadingStateTest: cameraSlider/adjustCurrentState.cpp cameraSliderTests/adjustCurrentStateTest.cpp
	g++ -DTEST -std=gnu++11 -Wall -IcameraSlider -IcameraSliderTests cameraSlider/adjustCurrentState.cpp cameraSliderTests/adjustCurrentStateTest.cpp -lcppunit -o adjustReadingStateTest
	
cameraSliderControllerTest: cameraSlider/cameraSliderController.cpp cameraSlider/cameraSliderController.h cameraSliderTests/cameraSliderControllerTest.cpp cameraSlider/userInput.h cameraSlider/HBridgeMotor.h
	g++ -DTEST -std=gnu++11 -Wall -g -IcameraSlider -IcameraSliderTests cameraSlider/cameraSliderController.cpp cameraSliderTests/cameraSliderControllerTest.cpp -lcppunit -o cameraSliderControllerTest