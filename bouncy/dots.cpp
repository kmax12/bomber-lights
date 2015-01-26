#include "dots.h"

#define LEDS_PER_STRIP 60

// define stuff
int num_dots = 1;
float time_since_draw = 0;
dot dots[MAX_DOTS];
bool collide[MAX_DOTS+1];

// quick function to return -1 or 1
static inline int random_sign() {
  if (random(2) > 1) {
    return 1;
  } else {
    return -1;
  }
}

// dot constructors
dot::dot() {
  position = random(LEFT, RIGHT);
  velocity = random(MIN_VEL, MAX_VEL) * random_sign();
  mass = random(MIN_MASS, MAX_MASS);
  radius = random(MIN_RAD, MAX_RAD);
  colorInd = (int)(random(NUM_COLORS));
}

dot::dot(float p, float v, float m, float r) {
  position = p;
  velocity = v;
  mass = m;
  radius = r;
  colorInd = (int)(random(NUM_COLORS));
}

dot::dot(float p, float v, float m, float r, int c) {
  position = p;
  velocity = v;
  mass = m;
  radius = r;
  colorInd = c;
}

// initial setup
void make_dots() {
  // seed arduino rng
  //randomSeed(analogRead(0));

  dots[0] = dot(2.0, 15.0, 1.0, 0.5, (int)random(180));
  Serial.print("v2: ");
  Serial.println(dots[0].velocity);
 // dots[1] = dot(40.0, -10.0, 2.0, 0.5, (int)random(180));

  /*
  for (int i = 0; i < num_dots; i++) {
    // create dot with random values
    dots[i] = dot();
  }
  */

  collide[0] = false;
  collide[num_dots] = false;
}

// called every frame to update the world state
void simulate_dots(float elapsed) {
  Serial.println(dots[0].velocity);
  Serial.println(dots[0].mass);
  // update position of each dot
  for (int i = 0; i < num_dots; i++) {
    //Serial.print("first: ");
    //Serial.println(dots[i].position);
    Serial.print("v: ");
    Serial.println(dots[i].velocity);
    Serial.println(dots[i].mass);
    
    dots[i].position += dots[i].velocity * elapsed;
        //Serial.print("second: ");
        //Serial.println(dots[i].position);
  }
 
  

  // check for walls
  if (WALL) {
    // left wall
    if (dots[0].position - dots[0].radius < LEFT && dots[0].velocity < 0) {
      dots[0].velocity *= -1;
    }

    // right wall
    if (dots[num_dots-1].position + dots[num_dots-1].radius > RIGHT &&
        dots[num_dots-1].velocity > 0) {
      dots[0].velocity *= -1;
    }
  }

  boolean c = false;

  // check for collision between each pair of dots
  for (int i = 1; i < num_dots; i++) {
    if ((dots[i].position + dots[i].radius > dots[i+1].position - dots[i+1].radius)){
      c = true;      
    }

    collide[i+1] = (dots[i].position + dots[i].radius > 
                    dots[i+1].position - dots[i+1].radius);
  }
  

  float new_vel[num_dots];

  // apply collisions to velocity of each dot
  float dm, cm;
  for (int i = 1; i < num_dots; i++) {
    new_vel[i] = dots[i].velocity;
        Serial.println(new_vel[i]);
//    if (collide[i] && collide[i+1]) {
//      new_vel[i] = 0;
//    } else if (collide[i]) {
//      dm = dots[i-1].mass - dots[i].mass;
//      cm = dots[i-1].mass + dots[i].mass;
//      new_vel[i] -= 2 * dots[i-1].mass * dots[i-1].velocity / cm - (dm * dots[i].velocity) / cm;
//    } else if (collide[i+1]) {
//      dm = dots[i].mass - dots[i+1].mass;
//      cm = dots[i].mass + dots[i+1].mass;
//      new_vel[i] -= 2 * dots[i-1].mass * dots[i-1].velocity / cm + 
//                    (dm * dots[i].velocity) / cm;
//    }
  }

  // check for special case
  for (int i = 1; i < num_dots; i++) {
    // For a 3-way collision, the squished ball's velocity has to make the old
    // momenta add up
    if (new_vel[i] == 0) {
      new_vel[i] = (
          dots[i-1].velocity * dots[i-1].mass +  // left old momentum
          dots[i].velocity * dots[i].mass +      // my old momentum
          dots[i+1].velocity * dots[i+1].mass -  // right old momentum
          new_vel[i-1] * dots[i-1].mass -         // left new momentum
          new_vel[i+1] * dots[i+1].mass) /        // right new momentum
            dots[i].mass;
    }
  }

  // actually update velocities
  for (int i = 0; i < num_dots; i++) {
    dots[i].velocity = new_vel[i];
  }
}

// Given an LED object, render dots to the strip
void draw_dots(int* leds) {
  int color;
  int pixel;
  
  for (int i = 0; i < LEDS_PER_STRIP; i++) {    
    leds[i] = 0;
  } 

  for (int i = 0; i < num_dots; i++) {    
    color = rainbow_colors[dots[i].colorInd];
    pixel = (int) dots[i].position;
    leds[pixel] = color;
  } 
}
