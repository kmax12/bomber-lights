#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <OctoWS2811.h>
#include "dots.h"
#include "colors.h"
#include "bouncy.h"

AudioInputAnalog         adc1(A4);           //xy=185,156
//AudioInputAnalog         adc2(A5);           //xy=203,271
AudioAnalyzeFFT1024      fft1024_1;      //xy=376,162
//AudioAnalyzeFFT1024      fft1024_2;      //xy=409,279
AudioConnection          patchCord1(adc1, fft1024_1);
//AudioConnection          patchCord2(adc2, fft1024_2);

DMAMEM int display_memory[TOTAL_LEDS*6];
int drawing_memory[TOTAL_LEDS*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(TOTAL_LEDS, display_memory, drawing_memory, config);

elapsedMillis elapsed_millis;
int led_buffer[TOTAL_LEDS];

elapsedMillis frame_millis;

bool first = true;

void setup()
{
  Serial.begin(9600);
  delay(1000);
   AudioMemory(24);
  //fft1024_1.windowFunction(AudioWindowHanning1024);
  //fft1024_2.windowFunction(AudioWindowHanning1024);
  leds.begin();
  
}

int last_buffer[TOTAL_LEDS];
int num_frames = 0;

void loop() {
  if (first) {
    first = false;
    elapsed_millis = 0;
    make_colors();
    make_dots();
    //Serial.println(dots[0].position);
  }
 
  //Serial.println(dots[0].velocity);
  // physics simulating
  Serial.println("b");
  simulate_dots((float)elapsed_millis / 1000.0);
  elapsed_millis = 0;
  time_since_draw -= DRAW_FRAME_TIME;

  // clear the board
  set_color(0,0,0, false);

  // render dots to the array led_buffer
  draw_dots(led_buffer);
  for (int i = 0; i < TOTAL_LEDS; i++) {
    // add motion blur by blending the current pixel array with the last one
    led_buffer[i] = blend_color(led_buffer[i], last_buffer[i]);
    leds.setPixel(i, led_buffer[i]);
    last_buffer[i] = dim_color(led_buffer[i], .7);
  } 

  // flush to the led strip
  if (!leds.busy()) {
    leds.show();
  }
  
  // framerate tracking
  num_frames += 1;
  if (frame_millis > 1000){
    //Serial.print("fps");
    //Serial.println((float)num_frames/frame_millis*1000);
    num_frames = 0;
    frame_millis = 0;
  }

  delay(10);
}

// Set the whole strip to a single rgb color
void set_color(int r, int g, int b, boolean show){
  for (int i = 0; i < TOTAL_LEDS; i++) {    
    leds.setPixel(i, r, g, b);    
  } 
  
  if (show) {
    leds.show();
  }
}
