#include "Arduino.h"
#include "ec11.h"
#include <Wire.h>
#include "var.h"
#include "libAdafruitNeoPixel.h"

EC11 encoder;
static int mode = 0;
static int color = 0;
static int tmode[3] = { 0,0,DEFAULT_INT };
boolean b_mode = false, b_color = false;
Adafruit_NeoPixel neopixels = Adafruit_NeoPixel(NBLED, PIN_LED, NEO_GRB + NEO_KHZ800);
uint32_t setcolor;
// Color
int colorTable[7][3] = { { 255,255,255 }, { 255,0,0 }, { 0,255,0 }, { 0, 0, 255 }, { 255,255,0 }, { 0,255,255 }, { 255, 0, 255 } };

//
// setup
//
void setup() {
#ifdef DEBUG
	Serial.begin(BAUDS_RATE);
	Serial.println("Init");
#endif // DEBUG

	pinMode(PIN_A, INPUT_PULLUP);
	pinMode(PIN_B, INPUT_PULLUP);
	pinMode(PIN_BUTTON, INPUT);
	pinMode(PIN_COLOR, INPUT);

	prepare();

	setcolor = neopixels.Color(colorTable[color][0], colorTable[color][1], colorTable[color][2], GAMMA);
	neopixels.setBrightness(tmode[2]);
	neopixels.begin();
	neopixels.show();
}

//
// loop
//
void loop() {
	ReadInput();
	ReadCoder();
	setcolor = neopixels.Color(colorTable[color][0], colorTable[color][1], colorTable[color][2], GAMMA);
	neopixels.setBrightness(tmode[2]);

	for (int i = 0; i <= tmode[0]; i++)
		neopixels.setPixelColor(i, 0);
	for (int i = 0 + tmode[0]; i <= tmode[1] + tmode[0]; i++)
		neopixels.setPixelColor(i, setcolor);
	for (int i = (tmode[1] + 1) + tmode[0]; i <= (NBLED - 1) + tmode[0]; i++)
		neopixels.setPixelColor(i, 0);
	neopixels.show();
}

//
// ReadCoder
//
void ReadCoder()
{
	EC11Event e;
	if (encoder.read(&e)) {
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
	//Read EC11 coder
	if (digitalRead(PIN_BUTTON) == HIGH && !b_mode) {
		b_mode = true;
		mode += 1;
		if (mode > 2)
			mode = 0;
	}
	if (digitalRead(PIN_BUTTON) == LOW)
		b_mode = false;

	//Read button color
	if (digitalRead(PIN_COLOR) == HIGH && !b_color) {
		b_color = true;
		color += 1;
		if (color > 6)
			color = 0;
	}
	if (digitalRead(PIN_COLOR) == LOW)
		b_color = false;
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