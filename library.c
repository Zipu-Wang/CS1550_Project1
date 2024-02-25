//header files
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <time.h>

#include "graphics.h"
#include "iso_font.h"

//macro to create a color value from r, g, b components
#define FB_DEVICE "/dev/fb0"
#define MAKE_COLOR(r, g, b) ((color_t) (r << 11) | (g << 5) | (b))

//16-bit color
typedef unsigned short color_t;
int desc, Dsize;
color_t* buffer;
struct fb_var_screeninfo SVinfo;
struct fb_fix_screeninfo SFinfo;
struct termios setting;

//open framebuffer device, configure screen, and map buffer
void init_graphics() {
	desc = open(FB_DEVICE, O_RDWR); 
    //print some error
    if(desc == -1) {
        perror("Error opening framebuffer");
        exit(1);
    }
//retrieving screen information using ioctl
    if(ioctl(desc, FBIOGET_VSCREENINFO, &SVinfo)) {
        perror("Error reading variable screen");
        close(desc);
        exit(1);
    }
    if(ioctl(desc, FBIOGET_FSCREENINFO, &SFinfo)) {
        perror("Error reading fixed screen");
        close(desc);
        exit(1);
    }
	Dsize = SVinfo.yres_virtual * SFinfo.line_length;

    if(ioctl(STDIN_FILENO, TCGETS, &setting)) {
        perror("Error getting settings");
        close(desc);
        exit(1);
    }	
    setting.c_lflag &= ~(ECHO | ICANON);
    if(ioctl(STDIN_FILENO, TCSETS, &setting)) {
        perror("Error setting settings");
        close(desc);
        exit(1);
    }
    //memory mapping the graphics buffer
	buffer = (color_t*) mmap(NULL, Dsize, PROT_WRITE, MAP_SHARED, desc, 0);

	clear_screen();
}

//clear screen and restore terminal settings
void exit_graphics() {
	clear_screen();

//restoring terminal settings
    if(ioctl(STDIN_FILENO, TCGETS, &setting)) {
        perror("Error getting settings");
    }

	setting.c_lflag |= ~ECHO;
	setting.c_lflag |= ~ICANON;

    if(ioctl(STDIN_FILENO, TCSETS, &setting)) {
        perror("Error restoring settings");
    }
	munmap(buffer, Dsize);
	
	close(desc);
}

//get a key press with a timeout
char getkey() {
	fd_set rfds; 
	struct timeval tw;

	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	
	tw.tv_sec = 2;
	tw.tv_usec = 0;

	int bufferChar = select(STDIN_FILENO+1, &rfds, NULL, NULL, &tw); 
	char bufferKey = '\0';

	if(bufferChar && bufferChar != -1) { 
		read(STDIN_FILENO, &bufferKey, sizeof(bufferKey));
	}

	return bufferKey;
}

//sleep for a specified number of milliseconds
void sleep_ms(long ms) {
	if(ms < 0) {
		return;
	}

	struct timespec req;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;

    nanosleep(&req, NULL);
}

//clear the terminal screen
void clear_screen() {
	write(STDOUT_FILENO, "\033[2J", 4);
}

//draw a pixel at specified coordinates with a color
void draw_pixel(int x, int y, color_t color) {
	if(x < 0 || y < 0 || x >= SVinfo.xres_virtual || y >= SVinfo.yres_virtual) {
		return;
	}

	int off = (y * SVinfo.xres_virtual) + x;
	color_t* px = buffer + off;
	*px = color;
}

//draw a rectangle with specified dimensions and color
void draw_rect(int x1, int y1, int width, int height, color_t c) {
    for(int x = x1; x < x1 + width; x++) {
        for(int y = y1; y < y1 + height; y++) {
            draw_pixel(x, y, c);
        }
    }
}

//draw a character at specified coordinates with a color
void draw_char(int x, int y, char ch, color_t c) {
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 8; j++) {
            if(iso_font[ch * 16 + i] & (1 << j)) {
                draw_pixel(x + j, y + i, c);
            }
        }
    }
}


//draw a string of text at specified coordinates
void draw_text(int x, int y, const char* text, color_t c) {
    int start = x;
    const int width = 8;

    for (int i = 0; text[i] != '\0'; i++) {
        draw_char(start, y, text[i], c);
        start += width;
    }
}

//draw a circle with specified center, radius, and color
void draw_circle(int x, int y, int r, color_t c) {
    int x1 = r - 1;
    int y1 = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (r << 1);

    while(x >= y) {
        draw_pixel(x + x1, y + y1, c);
        draw_pixel(x + y1, y + x1, c);
        draw_pixel(x - y1, y + x1, c);
        draw_pixel(x - x1, y + y1, c);
        draw_pixel(x - x1, y - y1, c);
        draw_pixel(x - y1, y - x1, c);
        draw_pixel(x + y1, y - x1, c);
        draw_pixel(x + x1, y - y1, c);

        if(err <= 0) {
            y1++;
            err += dy;
            dy += 2;
        }
        if(err > 0) {
            x1--;
            dx += 2;
            err += dx - (r << 1);
        }
    }
}
