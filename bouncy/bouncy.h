#ifndef BOUNCY_H
#define BOUNCY_H

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <OctoWS2811.h>
#include "dots.h"

#define LEDS_PER_STRIP 60

AudioInputAnalog         adc1(A4);           //xy=185,156
//AudioInputAnalog         adc2(A5);           //xy=203,271
AudioAnalyzeFFT1024      fft1024_1;      //xy=376,162
//AudioAnalyzeFFT1024      fft1024_2;      //xy=409,279
AudioConnection          patchCord1(adc1, fft1024_1);
//AudioConnection          patchCord2(adc2, fft1024_2);

DMAMEM int display_memory[LEDS_PER_STRIP*6];
int drawing_memory[LEDS_PER_STRIP*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(LEDS_PER_STRIP, display_memory, drawing_memory, config);

elapsedMillis elapsed_millis;

int rainbowColors[180];

bool first = true;

void setup();
void loop();

void set_color(int r, int g, int b, bool show);
int make_color(unsigned int hue, unsigned int saturation, unsigned int lightness);
unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue);

#endif
