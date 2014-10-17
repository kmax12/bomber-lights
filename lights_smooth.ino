// Example 48.1 - tronixstuff.com/tutorials > chapter 48 - 30 Jan 2013 
// MSGEQ7 spectrum analyser shield - basic demonstration
int strobe = 4; // strobe pins on digital 4
int res = 3; // reset pins on digital 5
int left[7]; // store band values in these arrays
int right[7];
int band;

int b = 6;
int g = 5;
int r = 7;

// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
const int numReadings = 3;
int index = 0;
int readingsRed[numReadings];      
int readingsGreen[numReadings];    
int readingsBlue[numReadings];     
int totalRed = 0;            
int totalGreen = 0;                  
int totalBlue = 0;                 
int averageRed = 0;              
int averageGreen = 0;                
int averageBlue = 0;  
int maxIntensity = 0;
int minIntensity = 10000;
float intensity = 1;

int time = millis();
int count = 0;
int mode = 0;
int c =0;

void setup(){
  Serial.begin(9600);
  pinMode(res, OUTPUT); // reset
  pinMode(strobe, OUTPUT); // strobe
  digitalWrite(res,LOW); // reset low
  digitalWrite(strobe,HIGH); //pin 5 is RESET on the shield
  analogWrite(r, 0);
  analogWrite(b, 0);
  analogWrite(g, 0);
  delay(100);
}


void loop(){
  readMSGEQ7();
  
   if((millis() - time) > 1000) {
      count += 1;
      
      if (count % 10 == 0){
        maxIntensity = 0;
        minIntensity = 10000;
      }
     
      
      if (count % 10 == 0){
        mode = (mode + 1) % 6;
      }
      
      if (count % 100 == 0){
        count = 0;
      }
      
    }
  
  intensity = min(altCalcIntensity(right)*.7 + intensity*.3, 1);
   //Serial.print(intensity);   Serial.print(" ");   Serial.print(minIntensity);   Serial.print(" ")+  Serial.println(maxIntensity); 
  //Serial.println(calcIntensity(right));
  if (intensity < .05f){
    rotate(right);
  } else{
    if (count % 30 < 31){
       rotateWithIntensity(right);
    } else {
      simpleSmoothMap(right);
    }
  }
  c += 1;
  //delay(10);
}

// Function to read 7 band equalizers
void readMSGEQ7(){
  digitalWrite(res, HIGH);
  digitalWrite(res, LOW);
  for(band=0; band <7; band++){
    digitalWrite(strobe,LOW); // strobe pin on the shield - kicks the IC up to the next band 
    delayMicroseconds(30); // 
    //left[band] = analogRead(1); // store left band reading
    right[band] = analogRead(0); // ... and the right
    digitalWrite(strobe,HIGH); 
  }
}

void writeRGB(int rVal, int bVal, int gVal){
  bVal = max(0, bVal - 50);
  rVal = min(255, rVal + 25);
  //gVal = max(0, gVal);
  Serial.print(rVal);  Serial.print(", ");  Serial.print(gVal);  Serial.print(", "); Serial.print(bVal);  Serial.println();  
  
  if (mode%3 == 0){
      analogWrite(b, bVal);
      analogWrite(r, rVal);
      analogWrite(g, gVal);  
  } else if(mode%3== 1){
      analogWrite(b, gVal);
      analogWrite(r, bVal);
      analogWrite(g, rVal);  
  } else{
      analogWrite(b, bVal);
      analogWrite(r, rVal);
      analogWrite(g, gVal);  
  }
}


void writeRGBSmooth(int rVal, int bVal, int gVal){
  int r_out = smooth(rVal, readingsRed, index, &totalRed, &averageRed, numReadings);
  int g_out = smooth(bVal, readingsGreen, index, &totalGreen, &averageGreen, numReadings);
  int b_out = smooth(gVal, readingsBlue, index, &totalBlue, &averageBlue, numReadings);
  writeRGB(r_out, g_out, b_out);
}

int displace = 250;
int displace2 = 500;
long t=0;
int periode = 5000;
int red = 0;
int blue = 0;
int green = 0;
float brightness;
int hue = 0;
float sat = 1;

void rotate(int spectrum[]){
  int rgb[3];  
  hsi2rgb(hue,sat,1, rgb);
  //Serial.println(hue);  
  writeRGBSmooth(rgb[0], rgb[1], rgb[2]);
  hue = (hue+1)%360;
  delay(10);
}

void rotateWithIntensity(int spectrum[]){
   int rgb[3];  
  hsi2rgb(hue,sat,intensity, rgb);
 
  //int red = min(255, (5+rgb[0])*(intensity));
  //int green = min(255, (rgb[1])*(intensity));
  //int blue = min(255,(rgb[2])*(intensity));
  
  int red  = rgb[0];
  int green  = rgb[1];
  int blue  = rgb[2];
  
  writeRGB(red,green,blue);
  
  
  if (c%12){
    hue = (hue+1)%360;
  }
}

int lastmax = 0;
int lastmaxcount = 0;

float altCalcIntensity(int spectrum[]){
  int total = 0;
  for(band=0; band <3; band++){
    total += spectrum[band]*(7-band)*(7-band);
  }
  if (total > maxIntensity){
    maxIntensity = total;
  }
  
  if (total < minIntensity){
    minIntensity = total;
  }
  
  //if (maxIntensity - minIntensity < 100){
  //  return 0.0f;
//  }

  if (total < lastmaxcount){
    total /=5;
  }
  else {
    Serial.println(millis());
    lastmax = total;
    lastmaxcount = 0;
  }
  
  lastmaxcount ++;
  if (lastmaxcount == 3){
    lastmaxcount = 0;
  }
  
  return (float) map(total, minIntensity, maxIntensity, 0, 25)/25.0;
}


float calcIntensity(int spectrum[]){
  int total = 0;
  for(band=0; band <3; band++){
    total += spectrum[band]*(7-band)*(7-band);
  }
  if (total > maxIntensity){
    maxIntensity = total;
  }
  
  if (total < minIntensity){
    minIntensity = total;
  }
  
  //if (maxIntensity - minIntensity < 100){
  //  return 0.0f;
//  }
  
  return (float) map(total, minIntensity, maxIntensity, 0, 25)/25.0;
}


void simpleSmoothMap(int spectrum[]){
  int low = spectrum[1];
  low = map(low, 40, 1000, 0, 255);

  int mid = spectrum[2];
  mid = map(mid, 40, 1000, 0, 255);

  int high = spectrum[4];
  high = map(high, 100, 1000, 0, 255);

  index += 1;
   
  if (index >= numReadings){
    index = 0;
  }
   
  writeRGBSmooth(low, mid, high);    
}

int smooth(int value, int readings[], int index, int* total, int* average, const int numReadings){
  *total = *total - *(readings+index); // subtract the last reading:
  *(readings+index) = value; // read from the sensor:  
  *total= *total + *(readings+index);      // add the reading to the total:
  //Serial.print(*total);   Serial.print(" ");   Serial.print(value);   Serial.println();
  // calculate the average:
  *average = *total / numReadings;  
  return *average;
}



// Function example takes H, S, I, and a pointer to the 
// returned RGB colorspace converted vector. It should
// be initialized with:
//
// int rgb[3];
//
// in the calling function. After calling hsi2rgb
// the vector rgb will contain red, green, and blue
// calculated values.

void hsi2rgb(float H, float S, float I, int* rgb) {
  int r, g, b;
  H = fmod(H,360); // cycle H around to 0-360 degrees
  H = 3.14159*H/(float)180; // Convert to radians.
  S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
  I = I>0?(I<1?I:1):0;
    
  // Math! Thanks in part to Kyle Miller.
  if(H < 2.09439) {
    r = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    g = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    b = 255*I/3*(1-S);
  } else if(H < 4.188787) {
    H = H - 2.09439;
    g = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    b = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    r = 255*I/3*(1-S);
  } else {
    H = H - 4.188787;
    b = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    r = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    g = 255*I/3*(1-S);
  }
  rgb[0]=r;
  rgb[1]=g;
  rgb[2]=b;
}
