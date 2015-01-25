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

const int FRAME_TIME = 17;
elapsedMillis delta;
elapsedMillis timeSinceDraw;

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

int colorInd = 0;
int color = rainbowColors[colorInd];

typedef struct dot {
  float position;
  float velocity;
  float mass;
  float radius;
  int colorInd;
} dot;

const int MAX_DOTS = 20;
int numDots = 5;
dot dots[MAX_DOTS];
// indicator for whether each neighboring pair of dots has collided in a given
// frame. buffered on either side by zeros.
bool collide[MAX_DOTS+1];

void make_dots() {
  for (int i = 0; i < numDots; i++) {
    // create dot with random values
    dots[i]->position = random(60);
    dots[i]->velocity = random(-30, 30);
    dots[i]->mass = random(10);
    dots[i]->radius = random(2.5);
    dots[i]->color = (int) random(180);
  }
  collide[0] = false;
  collide[MAX_DOTS] = false;
}

void loop()
{
  float l; 
  float r;
  int i;
  int c;
  
  if (timeSinceDraw > FRAME_TIME) {
    draw();
  }

  // update position of each dot
  for (int i = 0; i < numDots; i++) {
    dots[i].position += dots[i].velocity * elapsed;
  }

  // check for collision between each pair of dots
  for (int i = 1; i < numDots - 1; i++) {
    collide[i+1] = dots[i].position + dots[i].radius > dots[i+1].position - dots[i+1].radius;
  }

  int newVel[numDots];

  // apply collisions to velocity of each dot
  for (int i = 0; i < numDots; i++) {
    if (collide[i] && collide[i+1]) {
      newVel[i] = 0;
    } else if (collide[i]) {
      float dm = dots[i-1].mass - dots[i].mass;
      float cm = dots[i-1].mass + dots[i].mass;
      newVel[i] = 2 * dots[i-1].mass * dots[i-1].velocity / cm - (dm * dots[i].velocity) / cm;
    } else if (collide[i+1]) {
      float dm = dots[i].mass - dots[i+1].mass;
      float cm = dots[i].mass + dots[i+1].mass;
      newVel[i] = 2 * dots[i-1].mass * dots[i-1].velocity / cm - (dm * dots[i].velocity) / cm;
    } 
  }

  // check for special case
  for (int i = 0; i < numDots; i++) {
    // For a 3-way collision, the squished ball's velocity has to make the old
    // momenta add up
    if (newVel[i] == 0) {
      newVel[i] = (
          dots[i-1].velocity * dots[i-1].mass + 
          dots[i].velocity * dots[i].mass + 
          dots[i+1].velocity * dots[i+1].mass -
          newVel[i-1] * dots[i-1].mass - 
          newVel[i+1] * dots[i+1].mass) / dots[i].mass;
    }
  }

  // actually update velocities
  for (int i = 0; i < numDots; i++) {
    dots[i].velocity = newVel[i];
  }

  elapsed = 0;
}

void draw(){
  int color;
  int pixel;
  for (i = 0; i < MAX_DOTS; i++) {    
    color = rainbowColors[dots[i]->color];
    pixel = (int) dots[i]-->position;
    leds.setPixel(pixel, color)
    
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







