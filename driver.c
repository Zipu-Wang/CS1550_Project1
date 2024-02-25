#include "graphics.h"

int main() {
    init_graphics();

    clear_screen();

    draw_pixel(10, 10, MAKE_COLOR(31, 63, 31));

    draw_rect(20, 20, 50, 50, MAKE_COLOR(31, 0, 0));

    draw_text(100, 100, "Hello, Graphics!", MAKE_COLOR(0, 63, 0));

    draw_circle(150, 150, 30, MAKE_COLOR(0, 0, 31));

    char key;
    do {
        key = getkey();
    } while (key == '\0');

    sleep_ms(2000);

    exit_graphics();

    return 0;
}
