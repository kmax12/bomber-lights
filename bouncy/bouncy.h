#ifndef BOUNCY_H
#define BOUNCY_H

#define LEDS_PER_STRIP 60
#define NUM_STRIPS 1
#define TOTAL_LEDS (LEDS_PER_STRIP * NUM_STRIPS)

void set_color(int r, int g, int b, boolean show);

#endif
