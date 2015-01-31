#include "dots.h"

// define stuff
int num_dots = 0;
float time_since_draw = 0;
dot dots[MAX_DOTS];
float explosions[MAX_DOTS];
int frame = 0;

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
  dead = false;
}

dot::dot(float p, float v, float m, float r) {
  position = p;
  velocity = v;
  mass = m;
  radius = r;
  color_ind = (int)(random(NUM_COLORS));
  color_val = 1.0;
  dead = false;
}

dot::dot(float p, float v, float m, float r, int c) {
  position = p;
  velocity = v;
  mass = m;
  radius = r;
  color_ind = c;
  color_val = 1.0;
  dead = false;
}

// comparison function for qsort
bool cmpfunc (const dot& a, const dot& b) {
  return (a.position < b.position);
}

// initial setup
void make_dots() {
  // seed arduino rng
  //randomSeed(analogRead(0));

  // dot(position, velocity, mass, radius, color);
  dots[0] = dot(10.0, -10.0, 7.0, 0.5, 100);
  dots[1] = dot(20.0, 5.0, 1.0, 0.5, 50);
  dots[2] = dot(30.0, -0.0, 3.0, 0.5, 130);
  dots[3] = dot(45.0, -1.0, 6.0, 0.5, 170);  
  dots[4] = dot(50.0, 10.0, 5.0, 0.5, 20);
  dots[5] = dot(55.0, 5.0, 2.0, 0.5, 80);

  num_dots = 6;

  /*
  // create dots with random values
   for (int i = 0; i < num_dots; i++) {
   dots[i] = dot();
   }
   
   // sort the array so we can do collisions properly
   std::sort(dots, dots+num_dots, cmpfunc);
   */

  // initialize collide array to falses
  for (int i = 0; i < MAX_DOTS; i++){
    explosions[i] = -1;
  }
}

// called every frame to update the world state
void simulate_dots(float elapsed, int depth=0) {
  float new_vel[num_dots];
  float new_pos[num_dots];
  float new_color_val[num_dots];
  bool collide[num_dots+1];

  // make a tentative update of each dot
  for (int i = 0; i < num_dots; i++) {
    // copy old values into new_* arrays
    new_pos[i] = dots[i].position;
    new_vel[i] = dots[i].velocity;
    new_color_val[i] = dots[i].color_val;
    collide[i] = false;

    // update position and color_val for now
    new_pos[i] += dots[i].velocity * elapsed;
    new_color_val[i] = dots[i].color_val * std::pow(M_E, -COLOR_DECAY * elapsed);
  }

  // check for collisions with walls, and adjust position/velocity accordingly
  // left wall
  collide[0] = (new_pos[0] - dots[0].radius < LEFT);
  // right wall
  collide[num_dots] = (new_pos[num_dots-1] + dots[num_dots-1].radius > RIGHT);

  // check for collision between each pair of dots
  for (int i = 1; i < num_dots; i++) {
    collide[i] = (new_pos[i-1] + dots[i-1].radius > new_pos[i] - dots[i].radius);

    // if we have a 3-way collision or more, recurse with smaller values.
    if ((collide[i] && collide[i-1] || collide[i] && collide[i+1])
        && depth < MAX_RECURSION_DEPTH) {
      Serial.print("Recursing, depth: ");
      Serial.println(depth + 1);
      simulate_dots(elapsed, depth + 1);
      return;
    }
  }

  // apply collisions to calculate new position and velocity of each dot
  for (int r = 0; r < num_dots; r++) {
    // if this is the right edge, bounce off
    if (r == num_dots - 1 && collide[r+1]) {
      new_vel[num_dots-1] *= -1;
      new_pos[num_dots-1] += 2 * 
        (RIGHT - (new_pos[num_dots-1] + dots[num_dots-1].radius));
      continue;
    }

    // i is colliding with its left neighbor
    if (collide[r]) {      
      // does the chain include either one of the edges?
      bool left_wall = (r == 0);
      bool right_wall = false;

      // index of the leftmost ball in the chain.
      int l = max(r - 1, 0);

      float overlap, left_overlap, max_overlap_t = 0;

      // sum up mass + velocity
      for (r = l+1; r < num_dots && collide[r]; r++) {
        // find the earliest time that two balls began to overlap
        if (r > 0) {
          overlap = ((dots[r-1].position + dots[r-1].radius) - 
                    (dots[r].position - dots[r].radius));
          left_overlap = (overlap * dots[r-1].velocity) / 
                         (dots[r-1].velocity - dots[r].velocity);

          // now find how much time has elapsed since the collision
          max_overlap_t = max(max_overlap_t, left_overlap / dots[r-1].velocity);
        }
      }

      right_wall = (r == num_dots && collide[r]);
      r--;

      // if the chain includes a wall, calculate overlap with the wall
      // and invert total momentum
      // right_wall and left_wall should never both be true
      if (right_wall) {
        overlap = (RIGHT - (new_pos[num_dots-1] + dots[num_dots-1].radius));
        max_overlap_t = max(max_overlap_t, overlap / dots[num_dots-1].velocity);
        new_vel[l] = -abs(new_vel[l]);
      } else if (left_wall) {
        overlap = LEFT - (new_pos[0] - dots[0].radius);
        max_overlap_t = max(max_overlap_t, overlap / dots[0].velocity);
        new_vel[r] = abs(new_vel[r]);
      }

      // All dots have their positions adjusted back to the point of earliest
      // collision. Dots in the middle of the chain get smashed.
      for (int k = l; k <= r; k++) {
        if (k > l && k < r || 
            k == l && r > l && left_wall ||
            k == r && r > l && right_wall) {
          // destroy
          dots[k].dead = true;
        } else {
          new_color_val[k] += COLOR_INCR;
          new_pos[k] -= max_overlap_t * dots[k].velocity;
        }
      }

      // In the event of a chain reaction (3 or more balls colliding, or 2 
      // smashed together on a wall), the two dots on the outside act like 
      // they're bouncing off of each other.
      if (!left_wall && !right_wall) {
        float dm = dots[l].mass - dots[r].mass;
        float cm = dots[l].mass + dots[r].mass;

        new_vel[l] = dm * dots[l].velocity / cm + 
          2 * dots[r].mass * dots[r].velocity / cm;

        new_vel[r] = 2 * dots[l].mass * dots[l].velocity / cm -
          (dm * dots[r].velocity) / cm;
      }

      // update positions with new velocities
      for (int k = l; k <= r; k++) {
        new_pos[k] += (elapsed - max_overlap_t) * new_vel[k];
      }
    }
  }

  // do final update of positions & velocities
  for (int i = 0; i < num_dots; i++) {
    // never let velocity be exactly 0
    if (new_vel[i] == 0.0) {
      new_vel[i] = random(0.01, 0.02) * random_sign();
    }
    
    dots[i].velocity = new_vel[i];
    dots[i].position = new_pos[i];
    dots[i].color_val = min(new_color_val[i], 1);
  }
  
  // remove dead dots, keep array tight
  int e = 0;
  for (int i = 0; i < num_dots; i++) {
    if (dots[i].dead) {
      // spawn explosion
      explosions[e] = dots[i].position;
      e++;
      
      int j = i;
      num_dots--;
      
      while (j < num_dots) {
        dots[j] = dots[j+1];
        j++;
      }
      i--;
    }
  }
}

// render dots to the strip as an array of color values
void draw_dots(int* leds) {
  int color;
  int pixel;
  float pos_rem;
  int i;

  // reset strip
  for (i = 0; i < LEDS_PER_STRIP; i++) {    
    leds[i] = 0;
  } 
  
  // render dots
  for (i = 0; i < num_dots; i++) {
    color = rainbow_colors[dots[i].color_ind];
    pixel = (int) dots[i].position;
    pos_rem = dots[i].position - pixel;
    leds[pixel] = blend_color(leds[pixel], dim_color(color, dots[i].color_val * pow((1-pos_rem), 2)));
    if (pixel < RIGHT) {
      leds[pixel+1] = blend_color(leds[pixel+1], dim_color(color, dots[i].color_val * pow(pos_rem, 2)));
    }
  } 
  
  // render explosions
  i = 0;
  while (explosions[i] > 0) {
    float brightness;
    for (int pixel = (int) explosions[i] - 2; pixel <= (int)explosions[i] + 3; pixel++) {
      brightness = max(1 - abs(explosions[i] - pixel) / 3, 0);
      leds[pixel] = blend_color(leds[pixel], dim_color(0x00FFFFFF, pow(brightness, 2)));
    }
    explosions[i] = -1;
    i++;
  }
}
