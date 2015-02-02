#ifndef BOUNCY_H
#define BOUNCY_H

#define LEDS_PER_STRIP 60
#define NUM_STRIPS 1
#define TOTAL_LEDS (LEDS_PER_STRIP * NUM_STRIPS)
#define NUM_BANDS 5

void set_color(int r, int g, int b, boolean show);
void set_color(int color, boolean show);

#endif
