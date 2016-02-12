
/*  OctoWS2811 Rainbow.ino - Rainbow Shifting Test
 http://www.pjrc.com/teensy/td_libs_OctoWS2811.html
 Copyright (c) 2013 Paul Stoffregen, PJRC.COM, LLC
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 
 Required Connections
 --------------------
 pin 2:  LED Strip #1    OctoWS2811 drives 8 LED Strips.
 pin 14: LED strip #2    All 8 are the same length.
 pin 7:  LED strip #3
 pin 8:  LED strip #4    A 100 ohm resistor should used
 pin 6:  LED strip #5    between each Teensy pin and the
 pin 20: LED strip #6    wire to the LED strip, to minimize
 pin 21: LED strip #7    high frequency ringining & noise.
 pin 5:  LED strip #8
 pin 15 & 16 - Connect together, but do not use
 pin 4 - Do not use
 pin 3 - Do not use as PWM.  Normal use is ok.
 pin 1 - Output indicating CPU usage, monitor with an oscilloscope,
 logic analyzer or even an LED (brighter = CPU busier)
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <OctoWS2811.h>

#define LEDS_PER_STRIP 167
#define NUM_STRIPS 8
#define TOTAL_LEDS (LEDS_PER_STRIP * NUM_STRIPS)
#define NUM_COLORS 180

AudioInputAnalog         adc1(A4);           //xy=185,156
//AudioInputAnalog         adc2(A5);           //xy=203,271
AudioAnalyzeFFT1024      fft1;      //xy=376,162
//AudioAnalyzeFFT1024      fft1024_2;      //xy=409,279
AudioConnection          patchCord1(adc1, fft1);
//AudioConnection          patchCord2(adc2, fft1024_2);

DMAMEM int displayMemory[LEDS_PER_STRIP*6];
int drawingMemory[LEDS_PER_STRIP*6];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, config);

int rainbowColors[NUM_COLORS];
float intensity[NUM_COLORS];

void setup() {
  Serial.begin(9600);

  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);

  for (int i=0; i<NUM_COLORS; i++) {
    int hue = i * 2;
    int saturation = 100;
    int lightness = 50;
    // pre-compute the NUM_COLORS rainbow colors
    rainbowColors[i] = makeColor(hue, saturation, lightness);
    intensity[i] = 0;
  }

  AudioMemory(24);
  fft1.windowFunction(AudioWindowHanning1024);

  digitalWrite(1, LOW);
  leds.begin();
}


void loop() {
  rainbow(0);
}

// lower the RGB values of a color by a constant factor
int dim_color(int color, float value) {
  int r = (int)((color >> 16 & 0xFF) * value);
  int g = (int)((color >> 8 & 0xFF) * value);
  int b = (int)((color & 0xFF) * value);

  return (r << 16) | (g << 8) | b;    
}

// phaseShift is the shift between each row.  phaseShift=0
// causes all rows to show the same colors moving together.
// phaseShift=NUM_COLORS causes each row to be the opposite colors
// as the previous.
//
// cycleTime is the number of milliseconds to shift through
// the entire 360 degrees of the color wheel:
// Red -> Orange -> Yellow -> Green -> Blue -> Violet -> Red
//
void rainbow(int phaseShift)
{
  int color, index, x, y, offset;
  float decay = 0.5;
  float intensity[NUM_STRIPS];

  for (color=0; color < NUM_COLORS; color++) {
    digitalWrite(1, HIGH);

    for (x=0; x < LEDS_PER_STRIP; x++) {
      for (y=0; y < NUM_STRIPS; y++) {
        index = (color + x + y*phaseShift/2) % NUM_COLORS;

        if (fft1.available()) {
          intensity[y] = 0.5;//intensity[y] * (1 - decay) + decay * (float)(fft1.read(y * 50) * 1000) / 150.0;
        }

        leds.setPixel(x + y*LEDS_PER_STRIP, dim_color(rainbowColors[index], intensity[y]));
      }
    }

    leds.show();
    digitalWrite(1, LOW);
  }
}


