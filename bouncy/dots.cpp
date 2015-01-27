#include "dots.h"

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
  color_ind = (int)(random(NUM_COLORS));
  color_val = 1.0;
}

dot::dot(float p, float v, float m, float r) {
  position = p;
  velocity = v;
  mass = m;
  radius = r;
  color_ind = (int)(random(NUM_COLORS));
  color_val = 1.0;
}

dot::dot(float p, float v, float m, float r, int c) {
  position = p;
  velocity = v;
  mass = m;
  radius = r;
  color_ind = c;
  color_val = 1.0;
}

// comparison function for qsort
bool cmpfunc (const dot& a, const dot& b) {
  return (a.position < b.position);
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
  // create dots with random values
  for (int i = 0; i < num_dots; i++) {
    dots[i] = dot();
  }
  
  // sort the array so we can do collisions properly
  std::sort(dots, dots+num_dots, cmpfunc);
  */
  
  // initialize collide array to falses
  for (int i=0; i<MAX_DOTS+1; i++){
    collide[i] = false;
  }
}

// called every frame to update the world state
void simulate_dots(float elapsed) {
  float new_vel[num_dots];
  float new_pos[num_dots];
  float new_color_val[num_dots];
  float dm, cm, dist;

  // make a tentative update of each dot
  for (int i = 0; i < num_dots; i++) {
    // copy old values
    new_pos[i] = dots[i].position;
    new_vel[i] = dots[i].velocity;
    new_color_val[i] = dots[i].color_val;

    // update some for now
    new_pos[i] += dots[i].velocity * elapsed;
    new_color_val[i] = dots[i].color_val * std::pow(M_E, -COLOR_DECAY * elapsed);
  }
  Serial.println("a");
  // check for collisions with walls, and adjust position/velocity accordingly
  if (WALL) {
    // left wall
    if (new_pos[0] - dots[0].radius < LEFT) {
      new_vel[0] *= -1.0;
      new_pos[0] += 2 * (LEFT - (new_pos[0] - dots[0].radius));
      new_color_val[0] += COLOR_INCR;
    }

    // right wall
    if (new_pos[num_dots-1] + dots[num_dots-1].radius > RIGHT) {
      new_vel[num_dots-1] *= -1.0;
      new_pos[num_dots-1] += 2 * 
          (RIGHT - (new_pos[num_dots-1]+ dots[num_dots-1].radius));
      new_color_val[num_dots-1] += COLOR_INCR;
    }
  }

  // check for collision between each pair of dots, and recurse if there is a
  // 3-way ;)
  for (int i = 1; i < num_dots; i++) {
    collide[i] = (new_pos[i-1] + dots[i-1].radius > 
                    new_pos[i] - dots[i].radius);
    
    // if we have a 3-way collision or more, recurse with smaller values.
    // todo: should be possible to figure out which collision happened first and
    // just jump to that time
    if (collide[i] && collide[i-1]) {
      simulate_dots(elapsed / 2.0);
      simulate_dots(elapsed / 2.0);
      return;
    }
  }

  // apply initial position/velocity updates, since we're not recursing
  for (int i = 0; i < num_dots; i++) {
    dots[i].position = new_pos[i];
    dots[i].velocity = new_vel[i];
  }
  
  // apply collisions to calculate new position and velocity of each dot
  for (int i = 0; i < num_dots; i++) {
    /*
    if (collide[i] && collide[i+1]) { // this should never happen
      Serial.println("Error!!!!!!!!!!");
      exit(1);
      
    } else 
    */
    if (collide[i+1]) { // bounce the ball on the left (b1)
      Serial.print("collide left:");
      Serial.println(i);

      // calculate the left dot's new velocity
      dm = dots[i].mass - dots[i+1].mass;
      cm = dots[i].mass + dots[i+1].mass;
      new_vel[i] = 2 * dots[i+1].mass * dots[i+1].velocity / cm +
                    (dm * dots[i].velocity) / cm;

      // figure out where the left dot was when it collided
      float overlap = ((dots[i].position + dots[i].radius) - 
            (dots[i+1].position - dots[i+1].radius));
      float left_overlap = overlap * dots[i].velocity / 
            (dots[i].velocity - dots[i+1].velocity);
      new_pos[i] -= left_overlap;

      // now find how much time has elapsed since the collision, and apply the
      // new velocity for that long
      float ovl_t = left_overlap / dots[i].velocity;
      new_pos[i] += (elapsed - ovl_t) * new_vel[i];

      // increment brightness
      new_color_val[i] += COLOR_INCR;

    } else if (collide[i]) { // bounce the ball on the right (b2)
      Serial.print("collide right:");
      Serial.println(i);

      // reset collide[i]
      collide[i] = false;

      // calculate the right dot's new velocity
      dm = dots[i-1].mass - dots[i].mass;
      cm = dots[i-1].mass + dots[i].mass;
      new_vel[i] = 2 * dots[i-1].mass * dots[i-1].velocity / cm - 
            (dm * dots[i].velocity) / cm;
      
      // figure out where the right dot was when it actually collided
      float overlap = ((dots[i-1].position + dots[i-1].radius) -
            (dots[i].position - dots[i].radius));
      float right_overlap = overlap * dots[i].velocity / 
            (dots[i].velocity - dots[i-1].velocity);
      new_pos[i] += right_overlap;

      // now find how much time has elapsed since the collision, and apply the
      // new velocity for that long
      float ovl_t = right_overlap / dots[i].velocity;
      new_pos[i] += (elapsed - ovl_t) * new_vel[i];

      // increment brightness
      new_color_val[i] += COLOR_INCR;
    }
  }

  // do final update of positions & velocities
  for (int i = 0; i < num_dots; i++) {
    if (dots[i].velocity != new_vel[i]) {
      Serial.println("velocity diff:");
      Serial.println(dots[i].velocity);
      Serial.println(new_vel[i]);
    }

    dots[i].velocity = new_vel[i];
    dots[i].position = new_pos[i];
    dots[i].color_val = min(new_color_val[i],1);
  }
}

// render dots to the strip as an array of color values
void draw_dots(int* leds) {
  int color;
  int pixel;
  
  for (int i = 0; i < LEDS_PER_STRIP; i++) {    
    leds[i] = 0;
  } 

  for (int i = 0; i < num_dots; i++) {    
    color = rainbow_colors[dots[i].color_ind];
    pixel = (int) dots[i].position;
    leds[pixel] = blend_color(leds[pixel], dim_color(color, dots[i].color_val));
  } 
}
