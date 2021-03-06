#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <OctoWS2811.h>
#include "dots.h"
#include "colors.h"
#include "bouncy.h"

AudioInputAnalog         adc1(A4);           //xy=185,156
//AudioInputAnalog         adc2(A5);           //xy=203,271
AudioAnalyzeFFT256      fft1;      //xy=376,162
//AudioAnalyzeFFT1024      fft1024_2;      //xy=409,279
AudioConnection          patchCord1(adc1, fft1);
//AudioConnection          patchCord2(adc2, fft1024_2);
 
// TOTAL_LEDS = 1336
// 1336 * 4 * 2 * 6 = 10688B
DMAMEM int display_memory[LEDS_PER_STRIP * 6];
int drawing_memory[LEDS_PER_STRIP * 6];

const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(LEDS_PER_STRIP, display_memory, drawing_memory, config);

elapsedMillis elapsed_millis;

int led_buffer[TOTAL_LEDS];

elapsedMillis frame_millis;

bool first = true;


int led_location(int i) {
  int strip = i / LEDS_PER_STRIP;
  int base = strip * LEDS_PER_STRIP;
  
  if (strip == 0) {
    i += LEDS_PER_STRIP;
    base = LEDS_PER_STRIP;
    return base + (base + LEDS_PER_STRIP - i + 1);
  } else if (strip == 1) {
    i -= LEDS_PER_STRIP;
    return i;
  }
  
  if (strip % 2 == 0) {
    return base + (base + LEDS_PER_STRIP - i + 1);
  } else {
    return i;
  }
}

void setup()
{
  Serial.begin(9600);
  delay(1000);
  AudioMemory(24);
  fft1.windowFunction(AudioWindowHanning256);
  //fft1024_2.windowFunction(AudioWindowHanning1024);
  leds.begin();
}

int last_buffer[TOTAL_LEDS];
int num_frames = 0;
float intensity[NUM_BANDS];
int color = 0;

void loop() {
  if (first) {
    first = false;
    elapsed_millis = 0;
    frame_millis = 0;
    make_colors();
    make_dots();
  }

  simulate_dots((float)elapsed_millis / 1000.0, 0);
  elapsed_millis = 0;
  time_since_draw -= DRAW_FRAME_TIME;

  // clear the board
  set_color(0,0,0, false);
  
  // split the fft into 5 bands, totally arbitrarily
  if (fft1.available()) {
    for (int i = 0; i < NUM_BANDS; i++) {
      intensity[i] = (float)(fft1.read(6 * (i+1)) * 1000) / 350.0;
    }
  }

  // render dots to the array led_buffer
  draw_dots(led_buffer, color);
  
  // dots to pixels
  for (int i = 0; i < TOTAL_LEDS; i++) {
    // add motion blur by blending the current pixel array with the last one
    led_buffer[i] = blend_color(led_buffer[i], last_buffer[i]);
    
    // background pulsing, divided into five bands
    int band = (i * NUM_BANDS) / TOTAL_LEDS;
    
    //* do rainbow colors
    int band_color = (color + band * 20) % NUM_COLORS; 
    led_buffer[i] = blend_color(led_buffer[i], dim_color(rainbow_colors[band_color], intensity[band]));
    //*/
    
    leds.setPixel(led_location(i), led_buffer[i]);
    last_buffer[i] = dim_color(led_buffer[i], .9);
  } 

  // flush to the led strip
  if (!leds.busy()) {
    leds.show();
  }
  
  // framerate tracking
  num_frames += 1;
  if (frame_millis > 1000){
    Serial.print("fps");
    Serial.println((float)num_frames/frame_millis*1000);
    num_frames = 0;
    frame_millis = 0;    
    color = (color + 1) % NUM_COLORS;
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

// Set the whole strip to a single rgb color
void set_color(int color, boolean show){
  for (int i = 0; i < TOTAL_LEDS; i++) {    
    leds.setPixel(i, color);
  } 
  
  if (show) {
    leds.show();
  }
}
