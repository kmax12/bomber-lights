#include "bouncy.h"

void setup()
{
  Serial.begin(9600);
  
  // generate rainbow colors
  for (int i = 0; i < 180; i++) {
    int hue = i * 2;
    int saturation = 100;
    int lightness = 5;
    // pre-compute the 180 rainbow colors
    rainbowColors[i] = make_color(hue, saturation, lightness);
  }

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
  
  // drawing
  draw_dots((float)elapsed_millis / 1000.0);

  delay(10);
  elapsed_millis = 0;
}

void set_color(int r, int g, int b, boolean show){
  for (int i = 0; i < ledsPerStrip; i++) {    
    leds.setPixel(i, r, g, b);    
  }   
  
  if (show){
    leds.show();
  }
}

int make_color(unsigned int hue, unsigned int saturation, unsigned int lightness)
{
	unsigned int red, green, blue;
	unsigned int var1, var2;

	if (hue > 359) hue = hue % 360;
	if (saturation > 100) saturation = 100;
	if (lightness > 100) lightness = 100;

	// algorithm from: http://www.easyrgb.com/index.php?X=MATH&H=19#text19
	if (saturation == 0) {
		red = green = blue = lightness * 255 / 100;
	} else {
		if (lightness < 50) {
			var2 = lightness * (100 + saturation);
		} else {
			var2 = ((lightness + saturation) * 100) - (saturation * lightness);
		}
		var1 = lightness * 200 - var2;
		red = h2rgb(var1, var2, (hue < 240) ? hue + 120 : hue - 240) * 255 / 600000;
		green = h2rgb(var1, var2, hue) * 255 / 600000;
		blue = h2rgb(var1, var2, (hue >= 120) ? hue - 120 : hue + 240) * 255 / 600000;
	}
	return (red << 16) | (green << 8) | blue;
}

unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue)
{
	if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
	if (hue < 180) return v2 * 60;
	if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
	return v1 * 60;
}
