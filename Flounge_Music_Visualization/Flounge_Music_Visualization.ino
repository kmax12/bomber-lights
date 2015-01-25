#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <OctoWS2811.h>


AudioInputAnalog         adc1(A4);           //xy=185,156
//AudioInputAnalog         adc2(A5);           //xy=203,271
AudioAnalyzeFFT1024      fft1024_1;      //xy=376,162
//AudioAnalyzeFFT1024      fft1024_2;      //xy=409,279
AudioConnection          patchCord1(adc1, fft1024_1);
//AudioConnection          patchCord2(adc2, fft1024_2);


const int ledsPerStrip = 60;
DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

elapsedMillis reset;

int rainbowColors[180];

void setup()
{
  Serial.begin(9600);
  
  for (int i=0; i<180; i++) {
    int hue = i * 2;
    int saturation = 100;
    int lightness = 50;
    // pre-compute the 180 rainbow colors
    rainbowColors[i] = makeColor(hue, saturation, lightness);
  }
  
  AudioMemory(24);
  fft1024_1.windowFunction(AudioWindowHanning1024);
  //fft1024_2.windowFunction(AudioWindowHanning1024);
  leds.begin();
}

int a = 0;
int colorInd = 0;
int color = rainbowColors[colorInd];


void loop()
{
  float l; 
  float r;
  int i;
  int c;
  
  if(reset > 10)
  {
    reset = reset - 10;

    a++;
  }
  if(a >= 60)
  {
    a = 0;
    color = rainbowColors[colorInd];
    colorInd += 10;
    if(colorInd > 180){
      colorInd = 0;
    }
  }
  


  /*
  Serial.print(a);
   Serial.print(" ");
   Serial.print(b);
   Serial.print(" ");
   Serial.print(c);
   Serial.println();
   */

  if(fft1024_1.available())
    //if(!leds.busy())
  {
    // each time new FFT data is available
    // print it all to the Arduino Serial Monitor
    for (i=0; i<60; i++) {      
      l = fft1024_1.read(i) * 1000;
      //r = fft1024_2.read(i) * 1000;
      //n = random(0, 255);
      map(l, 0, 150, 0, 255);
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
       
      leds.setPixel(i, 0, 0, l);
    }

  }

  if(!leds.busy()){
    leds.show();
  }


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







