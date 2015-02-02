#ifndef COLORS_H
#define COLORS_H

#include "Arduino.h"

#define NUM_COLORS 180

extern int rainbow_colors[];

void make_colors();
int dim_color(int color, float value);
int blend_color(int c1, int c2);
int make_color(unsigned int hue, unsigned int saturation, unsigned int lightness);
unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue);

#endif
