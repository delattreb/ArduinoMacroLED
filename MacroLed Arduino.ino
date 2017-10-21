#include "ec11.h"
#include <Wire.h>
#include "var.h"
#include "libAdafruitNeoPixel.h"

EC11 encoder;
static int mode = 0;
static int tmode[3] = { 0,0,10 };
boolean b_mode = false;
Adafruit_NeoPixel neopixels = Adafruit_NeoPixel(NBLED, PIN_LED, NEO_GRB + NEO_KHZ800);
uint32_t color;


//
// setup
//
void setup() {
#ifdef DEBUG
	Serial.begin(BAUDS_RATE);
#endif // DEBUG

	pinMode(PIN_A, INPUT_PULLUP);
	pinMode(PIN_B, INPUT_PULLUP);
	pinMode(PIN_BUTTON, INPUT);
	prepare();

	color = neopixels.Color(254, 254, 254, GAMMA);
	neopixels.setBrightness(tmode[2]);
	neopixels.begin();
	neopixels.show();
}

//
// loop
//
void loop() {
	neopixels.setBrightness(tmode[2]);

	for (int i = 0; i <= tmode[0]; i++)
		neopixels.setPixelColor(i, 0);
	
	for (int i = 0 + tmode[0]; i <= tmode[1] + tmode[0]; i++)
		neopixels.setPixelColor(i, color);
	for (int i = (tmode[1] + 1) + tmode[0]; i <= (NBLED - 1) + tmode[0]; i++)
		neopixels.setPixelColor(i, 0);


	neopixels.show();

	ReadInput();
	ReadCoder();
	// Wait quite some time to demonstrate that we can check for events fairly infrequently and still not miss them.
	delay(200);
}

//
// ReadCoder
//
void ReadCoder()
{
	EC11Event e;
	if (encoder.read(&e)) {
		// OK, got an event waiting to be handled.

		if (e.type == EC11Event::StepCW) {
			if (mode == 2)
				tmode[mode] += e.count * MODE_2_MULT;
			else
				tmode[mode] += e.count;

			if (mode == 0)
				if (tmode[mode] + tmode[1] > MODE_0_MAX)
					tmode[mode] = tmode[mode] - e.count;
			if (mode == 1)
				if (tmode[mode] >= MODE_1_MAX)
					tmode[mode] = MODE_1_MAX;
			if (mode == 2)
				if (tmode[mode] >= MODE_2_MAX)
					tmode[mode] = MODE_2_MAX;
		}
		else {
			if (mode == 2)
				tmode[mode] -= e.count * MODE_2_MULT;
			else
				tmode[mode] -= e.count;

			if (mode == 0)
				if (tmode[mode] <= 0)
					tmode[mode] = 0;
			if (mode == 1)
				if (tmode[mode] <= 0)
					tmode[mode] = 0;
			if (mode == 2)
				if (tmode[mode] <= 10)
					tmode[mode] = 10;
		}
#ifdef DEBUG
		Serial.print("TMode: ");
		Serial.print(mode);
		Serial.print(" ");
		Serial.println(tmode[mode]);
#endif // DEBUG
	}
}

//
// ReadInput
//
void ReadInput()
{
	if (digitalRead(PIN_BUTTON) == HIGH && !b_mode) {
		b_mode = true;
		mode += 1;
		if (mode > 2)
			mode = 0;
#ifdef DEBUG
		Serial.print("Mode: ");
		Serial.println(mode);
#endif // DEBUG
	}
	if (digitalRead(PIN_BUTTON) == LOW)
		b_mode = false;
}

//
// pinDidChange
//
void pinDidChange() {
	encoder.checkPins(digitalRead(PIN_A), digitalRead(PIN_B));
}

//
// prepare
//
void prepare() {
	attachInterrupt(0, pinDidChange, CHANGE);
	attachInterrupt(1, pinDidChange, CHANGE);
}