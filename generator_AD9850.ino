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
byte current_step = STEP_100;
byte prev_step = STEP_NUM;
boolean byla_zmiana = false;
unsigned long czas_zmiany;

Adafruit_ILI9341 tft = Adafruit_ILI9341(tft_cs, tft_dc);
Bounce2::Button up = Bounce2::Button();
Bounce2::Button down = Bounce2::Button();
#define CLK 8
#define FQUP 9
#define BitData 10
AH_AD9850_B AD9850(CLK, FQUP, BitData);
long frequency = 7000000;
long step_value = 1000;
char bufor[] = "              ";              //zmienna pomocnicza do wyrzucania danych na tft
#define PIN_IN1 A0
#define PIN_IN2 A1
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR3);

void setup()
{
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
	delay(2000);
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
	    int enc = (int)(encoder.getDirection());
	    frequency = frequency + enc*step_value;
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
	tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
	tft.setTextSize(5);
	tft.setCursor(140, 94);
	tft.print(steps[current_step]);
}
void show_frequency(){
    long f_prefix = frequency/1000;                //pierwsza część częstotliwości
    long f_sufix = frequency%1000;                 //obliczamy resztę z częstotliwości
    sprintf(bufor,"%05lu",f_prefix);              //konwersja danych do wyświetlenia
    tft.setTextSize(5);
    tft.setCursor(4, 4);
    tft.print(bufor);
    tft.print(" ");
    sprintf(bufor,".%03lu",f_sufix);              //konwersja danych do wyświetlenia
    tft.print(bufor);
}
