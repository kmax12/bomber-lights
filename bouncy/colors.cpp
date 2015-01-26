#include "colors.h"
#include "Arduino.h"

// 180 pretty bright colors
int rainbow_colors[180];

// generate rainbow colors
void make_colors() {
   Serial.println("color enter");
  for (int i = 0; i < 180; i++) {
    int hue = i * 2;
    int saturation = 100;
    int lightness = 50;
    // pre-compute the 180 rainbow colors
    Serial.println("color loop");
    rainbow_colors[i] = make_color(hue, saturation, lightness);
  }
}

// convert hsv to rgb
int make_color(unsigned int hue, unsigned int saturation, unsigned int lightness) {
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

int dim_color(int color, float value) {
    int r = (int)((color >> 16 & 0xFF) * value);
    int g = (int)((color >> 8 & 0xFF) * value);
    int b = (int)((color & 0xFF) * value);
    
    return (r << 16) | (g << 8) | b;    
}

int blend_color(int c1, int c2) {
    int r = max(c1 >> 16 & 0xFF, c2 >> 16 & 0xFF);
    int g = max(c1 >> 8 & 0xFF, c2 >> 8 & 0xFF);
    int b = max(c1 & 0xFF, c2 & 0xFF);
    
    return (r << 16) | (g << 8) | b;
}

// convert hue value to rgb
unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue)
{
	if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
	if (hue < 180) return v2 * 60;
	if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
	return v1 * 60;
}
