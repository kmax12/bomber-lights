#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <OctoWS2811.h>
#include "dots.h"
#include "colors.h"

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

bool first = true;

void setup()
{
  Serial.begin(9600);

  make_colors();
  make_dots();
  
  AudioMemory(24);
  //fft1024_1.windowFunction(AudioWindowHanning1024);
  //fft1024_2.windowFunction(AudioWindowHanning1024);
  leds.begin();
}

void loop() {
  if (first) {
    first = false;
    elapsed_millis = 0;
  }

  // physics simulating
  simulate_dots((float)elapsed_millis / 1000.0);
  
  // check if it's time to draw yet
  time_since_draw += elapsed;
  if (time_since_draw >= DRAW_FRAME_TIME) {
    time_since_draw -= DRAW_FRAME_TIME;

    // clear the board
    set_color(0,0,0, false);

    // render dots
    draw_dots(leds);

    // flush to the led strip
    if (!leds.busy()) {
      leds.show();
    }
  }

  delay(10);
  elapsed_millis = 0;
}

// Set the whole strip to a single rgb color
void set_color(int r, int g, int b, boolean show){
  for (int i = 0; i < LEDS_PER_STRIP; i++) {    
    leds.setPixel(i, r, g, b);    
  } 
  
  if (show) {
    leds.show();
  }
}
