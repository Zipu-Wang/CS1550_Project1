#ifndef GRAPHICS_H
#define GRAPHICS_H

typedef unsigned short color_t;
#define MAKE_COLOR(r, g, b) ((color_t)(((r) << 11) | ((g) << 5) | (b)))

void init_graphics();
void exit_graphics();
void clear_screen();
char getkey();
void sleep_ms(long ms);
void draw_pixel(int x, int y, color_t color);
void draw_rect(int x1, int y1, int width, int height, color_t color);
void draw_text(int x, int y, const char* text, color_t color);
void draw_circle(int x0, int y0, int radius, color_t color);

#endif