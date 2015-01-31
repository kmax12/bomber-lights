#include "dots.h"

// define stuff
int num_dots = 0;
float time_since_draw = 0;
dot dots[MAX_DOTS];
bool collide[MAX_DOTS+1];
int frame = 0;

// quick function to return -1 or 1
static inline int random_sign() {
  if (random(2) > 1) {
    return 1;
  } 
  else {
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

  // dot(position, velocity, mass, radius, color);
  dots[0] = dot(10.0, -10.0, 7.0, 0.5, 100);
  dots[1] = dot(20.0, -20.0, 1.0, 0.5, 50);
  dots[2] = dot(30.0, -30.0, 3.0, 0.5, 130);
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
  for (int i=0; i<MAX_DOTS+1; i++){
    collide[i] = false;
  }
}

// called every frame to update the world state
void simulate_dots(float elapsed, int depth=0) {
  frame++;
  Serial.print("Frame ");
  Serial.println(frame);

  float new_vel[num_dots];
  float new_pos[num_dots];
  float new_color_val[num_dots];
  float time_left = elapsed;

  while (time_left > TIME_GRANULARITY) {
    elapsed = time_left;

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
    if (WALL) {
      // left wall
      collide[0] = (new_pos[0] - dots[0].radius < LEFT);

      // right wall
      collide[num_dots] = (new_pos[num_dots-1] + dots[num_dots-1].radius > RIGHT);
    }

    // check for collision between each pair of dots
    for (int i = 1; i < num_dots; i++) {
      collide[i] = (new_pos[i-1] + dots[i-1].radius > 
        new_pos[i] - dots[i].radius);

      // if we have a 3-way collision or more, recurse with smaller values.
      if ((collide[i] && collide[i-1] || collide[i] && collide[i+1])
        && elapsed > TIME_GRANULARITY * 2) {
        elapsed = elapsed / 2;
        Serial.print("Subdividing, time step: ");
        Serial.println(elapsed);
        continue;
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

        float total_mass = 0, total_momentum = 0, total_energy = 0, cm_velocity = 0;
        float overlap, left_overlap, max_overlap_t = 0;

        total_mass = dots[l].mass;
        total_momentum = dots[l].mass * dots[l].velocity;
        total_energy = total_momentum * dots[l].velocity;

        // sum up mass + velocity
        for (r = l+1; r < num_dots && collide[r]; r++) {
          total_mass += dots[r].mass;
          total_momentum += dots[r].mass * dots[r].velocity;
          total_energy += dots[r].mass * dots[r].velocity * dots[r].velocity;

          // find the earliest time that two balls began to overlap
          if (r > 0) {
            overlap = ((dots[r-1].position + dots[r-1].radius) - 
              (dots[r].position - dots[r].radius));
            left_overlap = overlap * dots[r-1].velocity / 
            (dots[r-1].velocity - dots[r].velocity);

            // now find how much time has elapsed since the collision
            max_overlap_t = max(max_overlap_t, left_overlap / dots[r-1].velocity);
          }
        }

        right_wall = (r == num_dots && collide[r]);
        r--;

        if (r - l > 1 || ((right_wall || left_wall) && r - l > 0)) {
          Serial.println("Colliding from, to:");
          Serial.println(l);
          Serial.println(r);
        }

        // this is the velocity of the whole system's center of mass,
        // post-collision
        cm_velocity = total_momentum / total_mass;

        // if the chain includes a wall, calculate overlap with the wall
        // and invert total momentum
        // right_wall and left_wall should never both be true
        if (right_wall) {
          overlap = (RIGHT - (new_pos[num_dots-1] + dots[num_dots-1].radius));
          max_overlap_t = max(max_overlap_t, overlap / dots[num_dots-1].velocity);

          for (int k = l; k <= r; k++) {
            new_vel[k] = -1 * abs(new_vel[k]);
          }
        } 
        else if (left_wall) {
          overlap = LEFT - (new_pos[0] - dots[0].radius);
          max_overlap_t = max(max_overlap_t, overlap / dots[0].velocity);

          for (int k = l; k <= r; k++) {
            new_vel[k] = abs(new_vel[k]);
          }
        }

        // All dots have their positions adjusted back to the point of earliest
        // collision.
        for (int k = l; k <= r; k++) {
          new_color_val[k] += COLOR_INCR;
          new_pos[k] -= max_overlap_t * dots[k].velocity;
        }

        // In the event of a chain reaction (3 or more balls colliding, or 2 
        // smashed together on a wall), the collision causes the dots in the 
        // middle to lose all their energy relative to the CM, and the two dots
        // on the outside act like they're bouncing off of each other.
        if (!left_wall && !right_wall) {
          // Set all velocities to that of the center of mass.
          // This is the final velocity for everything other than the two edges.
          for (int k = l; k <= r; k++) 
            new_vel[k] = cm_velocity;

          float dm = dots[l].mass - dots[r].mass;
          float cm = dots[l].mass + dots[r].mass;

          new_vel[l] = cm_velocity + 
            (dm * (dots[l].velocity - cm_velocity)) / cm + 
            2 * dots[r].mass * (dots[r].velocity - cm_velocity) / cm;

          new_vel[r] = cm_velocity + 
            2 * dots[l].mass * (dots[l].velocity - cm_velocity) / cm -
            (dm * (dots[r].velocity - cm_velocity)) / cm;

          // if the dots are still colliding with each other, jostle them a little
          if (new_pos[l] + dots[l].radius > new_pos[r] - dots[r].radius) {
            float avg_pos = (new_pos[l] + new_pos[r]) / 2.0;
            new_pos[l] = avg_pos - dots[l].radius * 1.1;
            new_pos[r] = avg_pos + dots[r].radius * 1.1;
            Serial.println(new_pos[l]);
            Serial.println(new_pos[r]);
            Serial.println(max_overlap_t * 1000);
          }

//          // if they're not moving away from each other, nudge them
//          if (new_vel[r] - new_vel[l] < 0.1) {
//            new_vel[l] -= 0.1 * dots[l].mass / cm;
//            new_vel[r] += 0.1 * dots[r].mass / cm; 
//          }
//
//          // cause things in the middle to drift away from each other just a little bit
//          float mid_pt = (float)(l + r) / 2.0;
//          for (int k = l+1; k < r; k++) {
//            new_vel[k] += (k - mid_pt) * 0.1;
//          }
        }

        // update positions with new velocities
        for (int k = l; k <= r; k++) {
          new_pos[k] += (elapsed - max_overlap_t) * new_vel[k];
        }
      }
    }

    // do final update of positions & velocities
    for (int i = 0; i < num_dots; i++) {
      //    if (dots[i].velocity != new_vel[i]) {
      //      Serial.println("velocity diff:");
      //      Serial.println(dots[i].velocity);
      //      Serial.println(new_vel[i]);
      //    }

      // never let velocity be exactly 0
      if (new_vel[i] == 0.0) {
        new_vel[i] = random(0.001, 0.002) * random_sign();
      }
      dots[i].velocity = new_vel[i];
      dots[i].position = new_pos[i];
      dots[i].color_val = min(new_color_val[i], 1);
    }
    time_left -= elapsed;
  }
}

// render dots to the strip as an array of color values
void draw_dots(int* leds) {
  int color;
  int pixel;
  float pos_rem;

  for (int i = 0; i < LEDS_PER_STRIP; i++) {    
    leds[i] = 0;
  } 

  for (int i = 0; i < num_dots; i++) {
    color = rainbow_colors[dots[i].color_ind];
    pixel = (int) dots[i].position;
    pos_rem = dots[i].position - pixel;
    leds[pixel] = blend_color(leds[pixel], dim_color(color, dots[i].color_val * pow((1-pos_rem), 2)));
    if (pixel < RIGHT) {
      leds[pixel+1] = blend_color(leds[pixel+1], dim_color(color, dots[i].color_val * pow(pos_rem, 2)));
    }
  } 
}

