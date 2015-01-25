#ifndef DOTS_H
#define DOTS_H

#include <time.h>
#include <stdlib.h>

#define LEDS_PER_STRIP 60
#define DRAW_FRAME_TIME 0.1
#define MAX_DOTS 20
#define NUM_COLORS 180

#define MIN_VEL 10
#define MAX_VEL 30
#define MIN_MASS 2
#define MAX_MASS 10
#define MIN_RAD 2
#define MAX_RAD 2

#define RANDOM(X, Y) (rand() * ((Y) - (X)) + (X))

#define LEFT 0
#define RIGHT 60
#define WALL 1

static inline int random_sign() {
  if (rand() * 2 > 1) {
    return 1;
  } else {
    return -1;
  }
}

// dot class 
class dot {
public:

  float position;
  float velocity;
  float mass;
  float radius;
  int colorInd;

  dot() {
    position = RANDOM(LEFT, RIGHT);
    velocity = RANDOM(MIN_VEL, MAX_VEL) * random_sign();
    mass = RANDOM(MIN_MASS, MAX_MASS);
    radius = RANDOM(MIN_RAD, MAX_RAD);
    colorInd = (int)(rand() * NUM_COLORS);
  }

  dot(float p, float v, float m, float r) {
    position = p;
    velocity = v;
    mass = m;
    radius = r;
    colorInd = (int)(rand() * NUM_COLORS);
  }

  dot(float p, float v, float m, float r, int c) {
    position = p;
    velocity = v;
    mass = m;
    radius = r;
    colorInd = c;
  }
};

// current number of active dots
int num_dots = 2;

// time since last draw
float time_since_draw = 0;

// active dots array
dot dots[MAX_DOTS];

// indicator for whether each neighboring pair of dots has collided in a given
// frame. buffered on either side by zeros.
bool collide[MAX_DOTS+1];

void make_dots();

void simulate_dots(float elapsed);

void draw_dots(float elapsed);

#endif
