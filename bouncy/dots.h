#ifndef DOTS_H
#define DOTS_H

#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "Arduino.h"
#include "colors.h"
#include "bouncy.h"

#define DRAW_FRAME_TIME 0.017
#define MAX_DOTS 20
#define NUM_COLORS 180

#define MIN_VEL 5
#define MAX_VEL 20
#define MIN_MASS 2
#define MAX_MASS 10
#define MIN_RAD 2
#define MAX_RAD 2

#define RANDOM(X, Y) (rand() * ((Y) - (X)) + (X))

#define LEFT 0
#define RIGHT TOTAL_LEDS
#define WALL 1

// macro to increase color value whenever there's a collision
#define COLOR_INCR 0.5
#define COLOR_DECAY 1

// dot class 
class dot {
public:

  float position;
  float velocity;
  float mass;
  float radius;
  int color_ind;
  float color_val;

  // constructor prototypes
  dot();
  dot(float p, float v, float m, float r);
  dot(float p, float v, float m, float r, int c);
};

// current number of active dots
extern int num_dots;

// time since last draw
extern float time_since_draw;

// active dots array
extern dot dots[];

// indicator for whether each neighboring pair of dots has collided in a given
// frame. buffered on either side by zeros.
extern bool collide[];

void make_dots();

void simulate_dots(float elapsed);

void draw_dots(int* led);

#endif
