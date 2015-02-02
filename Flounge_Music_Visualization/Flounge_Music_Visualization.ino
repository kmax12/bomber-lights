#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <OctoWS2811.h>


AudioInputAnalog         adc1(A4);           //xy=185,156
//AudioInputAnalog         adc2(A5);           //xy=203,271
AudioAnalyzeFFT1024      fft1;      //xy=376,162
//AudioAnalyzeFFT1024      fft1024_2;      //xy=409,279
AudioConnection          patchCord1(adc1, fft1);
//AudioConnection          patchCord2(adc2, fft1024_2);


#define LEDS_PER_STRIP 107
#define NUM_STRIPS 8
#define TOTAL_LEDS (LEDS_PER_STRIP * NUM_STRIPS)

DMAMEM int displayMemory[TOTAL_LEDS*6];
int drawingMemory[TOTAL_LEDS*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(TOTAL_LEDS, displayMemory, drawingMemory, config);

elapsedMillis reset;
elapsedMillis frame_millis;

int rainbowColors[180];
int led_location[TOTAL_LEDS];

void setup()
{
  Serial.begin(9600);
  
  /*
  int led;
  int base = 0;
  while (base < TOTAL_LEDS) {
    if (i & 2 == 1) {
      for (led = 0; led < LEDS_PER_STRIP; led++) {
        led_location[base + led] = base + LEDS_PER_STRIP - (led + 1);
      }
    } else {
      for (led = 0; led < LEDS_PER_STRIP; led++) {
        led_location[base + led] = base + led;
      }
    }
    
    base += LEDS_PER_STRIP;
  }
  */
    
  for (int i = 0; i < 180; i++) {
    int hue = i * 2;
    int saturation = 100;
    int lightness = 50;
    // pre-compute the 180 rainbow colors
    rainbowColors[i] = makeColor(hue, saturation, lightness);
  }
  
  AudioMemory(24);
  fft1.windowFunction(AudioWindowHanning1024);
  //fft1024_2.windowFunction(AudioWindowHanning1024);
  leds.begin();
}

int a = 0;
int num_frames = 0;
int colorInd = 0;
int color = rainbowColors[colorInd];


void loop()
{
  float l; 
  float r;
  int i;
  int c;
  
  a++;
  a = a % TOTAL_LEDS;

  /*
  if(fft1.available() && !leds.busy()) {
    // each time new FFT data is available
    // print it all to the Arduino Serial Monitor
    for (i = 0; i < TOTAL_LEDS; i++) { // Why was 60 here?
      // l = fft1.read(i) * 1000;
      //r = fft1024_2.read(i) * 1000;
      //n = random(0, 255);
      // map(l, 0, 150, 0, 255);
      //map(r, 0, 150, 0, 255);
      //Serial.print("a");
      //Serial.print(i);
      //Serial.println();

      //leds.setPixel(i, l, 0, 0);

      /*
      if(a > i & a - 5 < i)
       {
       leds.setPixel(i, color);
       }
       else
       {
         leds.setPixel(i, 0, 0, l);
       }
       */
       
       /*
       map(l, 0, 255, 0, 179);
       c = (int) l;
       if(0 > c)
       {
         c = 0;
       }
       if(180 < c)
       {
         c = 179;
       }
       //Serial.println(rainbowColors[c]);
       leds.setPixel(i, rainbowColors[c]);
       */
       
      //leds.setPixel(i, 0, 0, rainbowColors[100]);
    //}

  //}


  for (int i = 0; i < TOTAL_LEDS; i++) {
    leds.setPixel(i, 0);
  }
  
  leds.setPixel(a, rainbowColors[0]);

  if(!leds.busy()){
    leds.show();
  }
  
  // framerate tracking
  num_frames += 1;
  if (frame_millis > 1000){
    Serial.print("fps");
    Serial.println((float)num_frames/frame_millis*1000);
    num_frames = 0;
    frame_millis = 0;
  }
  
  // sleep 10 ms
  delay(10);
}



int makeColor(unsigned int hue, unsigned int saturation, unsigned int lightness)
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







