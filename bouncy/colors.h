#ifndef COLORS_H
#define COLORS_H

extern int[] rainbow_colors;

void make_colors();
int make_color(unsigned int hue, unsigned int saturation, unsigned int lightness);
unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue);

#endif
