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
void simulate_dots(float elapsed, int depth=0) {
  float dm, cm, dist;
  float new_vel[num_dots];
  float new_pos[num_dots];
  float new_color_val[num_dots];

  // make a tentative update of each dot
  for (int i = 0; i < num_dots; i++) {
    // copy old values into new_* arrays
    new_pos[i] = dots[i].position;
    new_vel[i] = dots[i].velocity;
    new_color_val[i] = dots[i].color_val;

    // update position and color_val for now
    new_pos[i] += dots[i].velocity * elapsed;
    new_color_val[i] = dots[i].color_val * std::pow(M_E, -COLOR_DECAY * elapsed);
  }
  Serial.println("a");

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
        // if we exceed recursion depth, assume they are simultaneous
        && depth < MAX_RECURSION_DEPTH) {
      simulate_dots(elapsed / 2.0, depth + 1);
      simulate_dots(elapsed / 2.0, depth + 1);
      return;
    }
  }

  // apply collisions to calculate new position and velocity of each dot
  for (int i = 0; i < num_dots; i++) {
    if (collide[i+1]) { 
      // there is a collision with i's right neighbor, so bounce the dot on the
      // left (d1).
      Serial.print("collide left:");
      Serial.println(i);

      // if there is a collision with the left neighbor too, this dot is
      // sandwiched, so nothing happens to it yet.
      if (collide[i]) {
        continue;
      }

      if (i + 1 == num_dots) { // hit the right wall
        new_vel[num_dots-1] *= -1.0;
        new_pos[num_dots-1] += 2 * 
            (RIGHT - (new_pos[num_dots-1] + dots[num_dots-1].radius));
        continue;
      } else {
        // if this is the leftmost ball in a chain, solve the chain too.
        float total_mass = dots[i].mass;
        float total_momentum = dots[i].mass * dots[i].velocity;
        float total_energy = total_momentum * dots[i].velocity;
        
        float overlap;
        float left_overlap;
        float max_overlap_t = 0;
        int j = i;

        // sum up mass + velocity
        while (j+1 < num_dots && collide[j+1]) {
          j++;
          total_mass += dots[j].mass;
          total_momentum += dots[j].mass * dots[j].velocity;
          total_energy += dots[j].mass * dots[j].velocity ** 2;

          // find the earliest time that two balls began to overlap
          overlap = ((dots[j-1].position + dots[j-1].radius) - 
                (dots[j].position - dots[j].radius));
          left_overlap = overlap * dots[j-1].velocity / 
                (dots[j-1].velocity - dots[j].velocity);

          // now find how much time has elapsed since the collision
          max_overlap_t = max(longest_overlap, left_overlap / dots[j-1].velocity);
        }

        // this is the velocity of the whole system's center of mass,
        // post-collision
        float cm_velocity = total_momentum / total_mass;

        // if the chain includes the right wall, invert momentum
        if (j+1 == num_dots && collide[num_dots]) {
          j++;
          overlap = (RIGHT - (new_pos[num_dots-1] + dots[num_dots-1].radius));
          max_overlap_t = max(longest_overlap, overlap / dots[num_dots-1].velocity);
          cm_velocity *= -1;
        }

        // all dots have their positions adjusted back to the point of earliest
        // collision. dots in the middle of the chain all get the same velocity
        // as the center of mass
        for (int k = i; k < j; k++) {
          new_vel[k] = cm_velocity;
          new_color_val[k] += COLOR_INCR;
          new_pos[k] -= max_overlap_t * dots[k].velocity;
        }

        // handle this case special to deal with colliding on the wall
        if (j < num_dots) {
          new_pos[j] -= max_overlap_t * dots[j].velocity;
        }
        
        // the collision acts as if the dots in the middle lost all their
        // energy, and the two on the outside bounce off each other.
        new_vel[i] = cm_velocity + 
                    2 * dots[j].mass * (dots[j].velocity - cm_velocity) / cm +
                    (dm * (dots[i].velocity - cm_velocity)) / cm;

        new_pos[k] += (elapsed - ovl_t) * new_vel[k];
      }

      // figure out where the left dot was when it collided
      float overlap = ((dots[i].position + dots[i].radius) - 
            (dots[i+1].position - dots[i+1].radius));
      float left_overlap = overlap * dots[i].velocity / 
            (dots[i].velocity - dots[i+1].velocity);
      new_pos[i] -= left_overlap;

      // now find how much time has elapsed since the collision, and apply the
      // new velocity for that long
      float ovl_t = left_overlap / dots[i].velocity;

    } else if (collide[i]) { // bounce the ball on the right (b2)
      Serial.print("collide right:");
      Serial.println(i);
      
      if (i == 0) { // hit the left wall
        new_vel[0] *= -1.0;
        new_pos[0] += 2 * (LEFT - (new_pos[0] - dots[0].radius));
        continue;
      }

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
