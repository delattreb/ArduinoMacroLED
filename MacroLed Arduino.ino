#include "Arduino.h"
#include "ec11.h"
#include <Wire.h>
#include "var.h"
#include "libAdafruitNeoPixel.h"

EC11 encoder;
static int mode = 0;
static int color = 0;
static int tablemode[3] = { 0,0,DEFAULT_INT };
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
#endif // DEBUG

	pinMode(PIN_A, INPUT_PULLUP);
	pinMode(PIN_B, INPUT_PULLUP);
	pinMode(PIN_BUTTON, INPUT);
	pinMode(PIN_COLOR, INPUT);

	prepare();

	setcolor = neopixels.Color(colorTable[color][0], colorTable[color][1], colorTable[color][2], GAMMA);
	neopixels.setBrightness(tablemode[2]);
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
	neopixels.setBrightness(tablemode[2]);

	//Switch off pixel
	for (int i = 0; i <= NBLED - 1; i++)
		neopixels.setPixelColor(i, 0);
	//Draw pixel
	for (int i = 0; i <= tablemode[1]; i++)
		if (i + tablemode[0] >= NBLED)
			neopixels.setPixelColor((i + tablemode[0]) - (NBLED), setcolor);
		else
			neopixels.setPixelColor(i + tablemode[0], setcolor);
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
			if (mode == 0)
				tablemode[mode] = (tablemode[mode] + e.count) % NBLED;
			if (mode == 1)
			{
				tablemode[mode] += e.count;
				if (tablemode[mode] >= NBLED - 1)
					tablemode[mode] = NBLED - 1;
			}
			if (mode == 2) {
				tablemode[mode] += e.count * MODE_2_MULT;
				if (tablemode[mode] >= MODE_2_MAX)
					tablemode[mode] = MODE_2_MAX;
			}
		}
		else {
			if (mode == 0)
				if (tablemode[mode] - e.count < 0)
					tablemode[mode] = NBLED - e.count;
				else
					tablemode[mode] = (tablemode[mode] - e.count) % NBLED;
			if (mode == 1) {
				tablemode[mode] -= e.count;
				if (tablemode[mode] <= 0)
					tablemode[mode] = 0;
			}
			if (mode == 2) {
				tablemode[mode] -= e.count * MODE_2_MULT;
				if (tablemode[mode] <= MODE_2_MIN)
					tablemode[mode] = MODE_2_MIN;
			}
		}
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