#include "ec11.h"

EC11 encoder;

const int encoderPinA = 2;
const int encoderPinB = 3;
const int encoderButton = 6;
boolean b_mode = false;

void pinDidChange() {
	encoder.checkPins(digitalRead(encoderPinA), digitalRead(encoderPinB));
}

void prepare() {
	attachInterrupt(0, pinDidChange, CHANGE);
	attachInterrupt(1, pinDidChange, CHANGE);
}

void setup() {
	Serial.begin(9600);
	pinMode(encoderPinA, INPUT_PULLUP);
	pinMode(encoderPinB, INPUT_PULLUP);
	pinMode(encoderButton, INPUT);
	prepare();
}

static int value = 0;
static int mode = 0;

void loop() {
	if (digitalRead(encoderButton) == HIGH && !b_mode) {
		b_mode = true;
		mode += 1;
		if (mode > 2)
			mode = 0;
		Serial.println("Push");
	}
	if (digitalRead(encoderButton) == LOW)
		b_mode = false;


	EC11Event e;
	if (encoder.read(&e)) {
		// OK, got an event waiting to be handled.

		if (e.type == EC11Event::StepCW) {
			value += e.count;
		}
		else {
			value -= e.count;
		}
		Serial.println(value);
	}
	// Wait quite some time to demonstrate that we can check for events fairly infrequently and still not miss them.
	delay(200);
}
