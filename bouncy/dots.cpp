#include "dots.h"

#define LEDS_PER_STRIP 60

// define stuff
int num_dots = 6;
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
  color_val = 1.0;
}

dot::dot(float p, float v, float m, float r) {
  position = p;
  velocity = v;
  mass = m;
  radius = r;
  colorInd = (int)(random(NUM_COLORS));
  color_val = 1.0;
}

dot::dot(float p, float v, float m, float r, int c) {
  position = p;
  velocity = v;
  mass = m;
  radius = r;
  colorInd = c;
  color_val = 1.0;
}

// initial setup
void make_dots() {
  // seed arduino rng
  //randomSeed(analogRead(0));

  dots[0] = dot(20.0, 15.0, 10.0, 0.5, 100);
  dots[1] = dot(23.0, -100.0, 1.0, 0.5, 50);
  dots[2] = dot(30.0, 1.0, 3.0, 0.5, 130);
  dots[3] = dot(35.0, 8.0, 6.0, 0.5, 170);  
  dots[4] = dot(45.0, 10.0, 5.0, 0.5, 20);
  dots[5] = dot(52.0, -10.0, 2.0, 0.5, 80);

  num_dots = 3;
  
/*
  for (int i = 0; i < num_dots; i++) {
    // create dot with random values
    dots[i] = dot();
  }
  
  qsort(
  */
  
  for (int i=0; i<MAX_DOTS+1; i++){
    collide[i] = false;
  }
}

int cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}

// called every frame to update the world state
void simulate_dots(float elapsed) {
  Serial.println(random(90,100)/100.0);
  elapsed *= random(50,100)/100.0;
//  Serial.println(dots[0].mass);
  // update position of each dot
  for (int i = 0; i < num_dots; i++) {
    dots[i].position += dots[i].velocity * elapsed;
    dots[i].color_val *= 0.9;
  }
  
  // check for walls
  if (WALL) {
    // left wall
    if (dots[0].position - dots[0].radius < LEFT) {
      dots[0].velocity *= -1.0;
      dots[0].position += 2*(LEFT - (dots[0].position - dots[0].radius));
      dots[0].color_val += 0.5;
    }

    // right wall
    if (dots[num_dots-1].position + dots[num_dots-1].radius > RIGHT) {
      dots[num_dots-1].velocity *= -1.0;
      dots[num_dots-1].position += 2*(RIGHT - (dots[num_dots-1].position+dots[num_dots-1].radius));
      dots[num_dots-1].color_val += 0.5;
    }
  }

  boolean c = false;

  // check for collision between each pair of dots
  for (int i = 1; i < num_dots; i++) {
    if ((dots[i].position + dots[i].radius > dots[i+1].position - dots[i+1].radius)){
      c = true;      
    }

    collide[i] = (dots[i-1].position + dots[i-1].radius > 
                    dots[i].position - dots[i].radius);
  }
  
  float new_vel[num_dots];

  // apply collisions to velocity of each dot
  float dm, cm, dist;
  for (int i = 0; i < num_dots; i++) {
    new_vel[i] = dots[i].velocity;
    
    if (collide[i] && collide[i+1]) {
      new_vel[i] = 0;
      dots[i].color_val += 0.5;
      
    } else if (collide[i]) { // bounce the ball on the right (b2)
      Serial.println("collide i");
      collide[i] = false;
      dm = dots[i-1].mass - dots[i].mass;
      cm = dots[i-1].mass + dots[i].mass;
      new_vel[i] = 2 * dots[i-1].mass * dots[i-1].velocity / cm - (dm * dots[i].velocity) / cm;
      
      float overlap = ((dots[i-1].position + dots[i-1].radius) - (dots[i].position - dots[i].radius));
      float right_overlap = overlap * dots[i].velocity / (dots[i].velocity - dots[i-1].velocity);
      
      dots[i].position += right_overlap;
      float ovl_time = right_overlap / dots[i].velocity;
      dots[i].position += (elapsed - ovl_time) * new_vel[i];
      dots[i].color_val += 0.5;
      
    } else if (collide[i+1]) { // bounce the ball on the left (b1)
      Serial.println("collide i +1");
      dm = dots[i].mass - dots[i+1].mass;
      cm = dots[i].mass + dots[i+1].mass;
      new_vel[i] = 2 * dots[i+1].mass * dots[i+1].velocity / cm +
                    (dm * dots[i].velocity) / cm;
                    
      float overlap = ((dots[i].position + dots[i].radius) - (dots[i+1].position - dots[i+1].radius));
      float left_overlap = overlap * dots[i].velocity / (dots[i].velocity - dots[i+1].velocity);
      
      dots[i].position -= left_overlap;
      float ovl_time = left_overlap / dots[i].velocity;
      dots[i].position += (elapsed - ovl_time) * new_vel[i];
      dots[i].color_val += 0.5;

    }
  }

  // check for special case
//  for (int i = 1; i < num_dots; i++) {
//    // For a 3-way collision, the squished ball's velocity has to make the old
//    // momenta add up
//    if (new_vel[i] == 0) {
//      new_vel[i] = (
//          dots[i-1].velocity * dots[i-1].mass +  // left old momentum
//          dots[i].velocity * dots[i].mass +      // my old momentum
//          dots[i+1].velocity * dots[i+1].mass -  // right old momentum
//          new_vel[i-1] * dots[i-1].mass -         // left new momentum
//          new_vel[i+1] * dots[i+1].mass) /        // right new momentum
//            dots[i].mass;
//    }
//  }

  // actually update velocities
  for (int i = 0; i < num_dots; i++) {
    if (dots[i].velocity != new_vel[i]) {
      Serial.println("diff:");
      Serial.println(dots[i].velocity);
      Serial.println(new_vel[i]);
    }
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
    leds[pixel] = dim_color(color, dots[i].color_val);
  } 
}
