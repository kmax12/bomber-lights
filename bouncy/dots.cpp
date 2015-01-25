#include "dots.h"

void make_dots() {
  dots[0] = dot(20.0, 15.0, 1.0, 0.5, (int)random(180));
  dots[1] = dot(40.0, -10.0, 2.0, 0.5, (int)random(180));

  /*
  for (int i = 0; i < num_dots; i++) {
    // create dot with random values
    dots[i] = dot();
  }
  */

  collide[0] = false;
  collide[num_dots] = false;
}

void simulate_dots(float elapsed) {
  // update position of each dot
  for (int i = 0; i < num_dots; i++) {
    dots[i].position += dots[i].velocity * elapsed;
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
  
  // indicate collision with a flash
  if (c) {
    set_color(50,50,50, true);
  }

  float new_vel[num_dots];

  // apply collisions to velocity of each dot
  float dm, cm;
  for (int i = 1; i < num_dots; i++) {
    new_vel[i] = dots[i].velocity;
    if (collide[i] && collide[i+1]) {
      new_vel[i] = 0;
    } else if (collide[i]) {
      dm = dots[i-1].mass - dots[i].mass;
      cm = dots[i-1].mass + dots[i].mass;
      new_vel[i] -= 2 * dots[i-1].mass * dots[i-1].velocity / cm - (dm * dots[i].velocity) / cm;
    } else if (collide[i+1]) {
      dm = dots[i].mass - dots[i+1].mass;
      cm = dots[i].mass + dots[i+1].mass;
      new_vel[i] -= 2 * dots[i-1].mass * dots[i-1].velocity / cm + (dm * dots[i].velocity) / cm;
    }
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

void draw_dots(float elapsed) {
  // check if it's time to draw yet
  time_since_draw += elapsed;
  if (time_since_draw < DRAW_FRAME_TIME)
    return;

  time_since_draw -= DRAW_FRAME_TIME;
  set_color(0,0,0, false);
  int color;
  int pixel;

  for (int i = 0; i < num_dots; i++) {    
    color = rainbowColors[dots[i].colorInd];
    pixel = (int) dots[i].position;
    leds.setPixel(pixel, color);
  } 

  if (!leds.busy()) {
    leds.show();
  }
}
