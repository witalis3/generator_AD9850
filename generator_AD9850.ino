/*
 * generator na układzie AD9850
 * na bazie płytki PCB od sterownika PA 1.0
 *
 *
 *
 */
#include "generator_AD9850.h"
#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "Bounce2.h"
#include <AH_AD9850_B.h>
#include <RotaryEncoder.h>

const char tft_cs = 2;			// <= /CS pin (chip-select, LOW to get attention of ILI9341, HIGH and it ignores SPI bus) default 10!
const char tft_dc = 3;			// <= DC pin (1=data or 0=command indicator line) also called RS
#define STEP_UP_PIN			A2
#define STEP_DOWN_PIN	7

enum
{
	STEP_1,
	STEP_10,
	STEP_100,
	STEP_1000,
	STEP_10000,
	STEP_100000,
	STEP_1000000,
	STEP_NUM
};
const char* steps[STEP_NUM] = {"   1Hz", "  10Hz", " 100Hz", "  1kHz", " 10kHz", "100kHz", "  1MHz"};
byte current_step = STEP_1000;
byte prev_step = STEP_NUM;
boolean byla_zmiana = false;
unsigned long czas_zmiany;

Adafruit_ILI9341 tft = Adafruit_ILI9341(tft_cs, tft_dc);
Bounce2::Button up = Bounce2::Button();
Bounce2::Button down = Bounce2::Button();
#define CLK 4
#define FQUP 9
#define BitData 10
AH_AD9850_B AD9850(CLK, FQUP, BitData);
long frequency = 7000000;
const long low_frequency_limit = 100;             //dolny limit częstotliwości
const long high_frequency_limit = 40000000;            //górny limit częstotliwości
long step_value = 1000;
char bufor[] = "              ";              //zmienna pomocnicza do wyrzucania danych na tft
#define PIN_IN1 A1
#define PIN_IN2 A0
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR3);

void setup()
{
	Serial.begin(115200);
	up.attach(STEP_UP_PIN, INPUT_PULLUP);
	up.interval(5);
	up.setPressedState(LOW);
	down.attach(STEP_DOWN_PIN, INPUT_PULLUP);
	down.interval(5);
	down.setPressedState(LOW);

	tft.begin();
	tft.setRotation(3);
	tft.fillScreen(ILI9341_BLACK);
	tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
	tft.setTextSize(2);
	tft.setCursor(2, 90);
	tft.print(" Generator AD9850 ver. 1.0");
	delay(1000);
	tft.fillScreen(ILI9341_BLACK);
	show_template();
	switch_steps();
	AD9850.set_frequency(frequency);
	show_frequency();
}
void loop()
{
	  static int pos = 0;
	  encoder.tick();
	  int newPos = encoder.getPosition();
	  if (pos != newPos)
	  {
	    int enc = (int)encoder.getDirection();
	    Serial.print("enc: ");
	    Serial.println(enc);
	    Serial.print("step_value: ");
	    Serial.println(step_value);
	    frequency = frequency + enc*step_value;
	    frequency = constrain(frequency, low_frequency_limit, high_frequency_limit);
	    AD9850.set_frequency(frequency);
		show_frequency();
	    pos = newPos;
	  }
	up.update();
	if (up.pressed())
	{
		if (current_step == STEP_1000000)
		{
			current_step = STEP_1;
		}
		else
		{
			current_step++;
		}
		byla_zmiana = true;
		czas_zmiany = millis();
		switch_steps();
		delay(200);
	}
	down.update();
	if (down.pressed())
	{
		if (current_step == STEP_1)
		{
			current_step = STEP_1000000;
		}
		else
		{
			current_step--;
		}
		byla_zmiana = true;
		czas_zmiany = millis();
		switch_steps();
		delay(200);
	}
}
void show_template()
{
	tft.drawRoundRect(0, 0, 320, 60, 5, ILI9341_WHITE);	// ramka dla pasma
	tft.setCursor(4, 66);
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.setTextSize(4);
	tft.setCursor(0, 94);
	tft.print("Step:");
}
void switch_steps()
{
	prev_step = current_step;
	step_value = get_step_value(current_step);
	tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
	tft.setTextSize(5);
	tft.setCursor(140, 94);
	tft.print(steps[current_step]);
}
void show_frequency()
{
	Serial.print("frequency: ");
	Serial.println(frequency);
    tft.setTextSize(5);
    tft.setCursor(30, 12);
    long f_prefix = frequency/1000;
	Serial.print("f_prefix: ");
	Serial.println(f_prefix);
    sprintf(bufor,"%05lu",f_prefix);
    tft.print(bufor);
    long f_sufix = frequency%1000;
	Serial.print("f_sufix: ");
	Serial.println(f_sufix);
    sprintf(bufor,".%03lu",f_sufix);
    tft.print(bufor);
}
long get_step_value(byte step)
{
	long zwrot;
	switch (step) {
		case STEP_1:
			zwrot = 1;
			break;
		case STEP_10:
			zwrot = 10;
			break;
		case STEP_100:
			zwrot = 100;
			break;
		case STEP_1000:
			zwrot = 1000;
			break;
		case STEP_10000:
			zwrot = 10000;
			break;
		case STEP_100000:
			zwrot = 100000;
			break;
		case STEP_1000000:
			zwrot = 1000000;
			break;
		default:
			zwrot = 1000;
			break;
	}
	return zwrot;
}
