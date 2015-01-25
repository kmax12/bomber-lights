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

void setup()
{
  Serial.begin(9600);
  Serial.print("fuuuuck");
  AudioMemory(24);
  fft1024_1.windowFunction(AudioWindowHanning1024);
  //fft1024_2.windowFunction(AudioWindowHanning1024);
  leds.begin();
}

int a = 0;

void loop()
{
  float l; 
  float r;
  int i;
  
  
  if(reset > 500)
  {
    reset = reset - 500;

    a++;
  }
  if(a >= 60)
  {
    a = 0;
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

      leds.setPixel(i, l, 0, 0);

      
      if(a > i)
       {
       Serial.println("red");
       leds.setPixel(i, l, 0, 0);
       }
       else
       {
       Serial.println("green");
       leds.setPixel(i, 0, l, 0);
       }
       
      //leds.setPixel(i, 0, 0, l);
    }

  }

  if(!leds.busy()){
    leds.show();
  }


}









