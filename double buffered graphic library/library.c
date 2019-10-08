//
// Created by zhuan on 1/25/2018.
//
# include <stdio.h>         // defining EOF value
# include <fcntl.h>
#include <linux/fb.h>
# include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
# include "library.h"

#define MAX_MEM 1<<30	//define the max size to 2^30

static void* panel = NULL;
int fd;
int x_size, y_size,buffer_size;
struct fb_var_screeninfo variable_info;
struct fb_fix_screeninfo fixed_info;

void init_graphics(){
    fd = open("/dev/fb0",O_RDWR);
    ioctl (fd, FBIOGET_VSCREENINFO, &variable_info);
    ioctl (fd, FBIOGET_FSCREENINFO, &fixed_info);
    y_size = variable_info.yres_virtual;
    x_size = fixed_info.line_length;
    buffer_size = x_size * y_size;
    panel = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    struct termios t;
    ioctl(0,TCGETS,&t);
    t.c_lflag &= ~ECHO;
    t.c_lflag &= ~ICANON;
    ioctl(0,TCSETS,&t);
};

void exit_graphics(){
    struct termios t;
    ioctl(0,TCGETS,&t);
    t.c_lflag |= ECHO;
    t.c_lflag |= ICANON;
    ioctl(0,TCSETS,&t);
    close(fd);
}

void clear_screen(){
    char msg[] = "\033[2J";
    write(1,msg,sizeof(msg)-1);
}

char getkey(){
    fd_set fds_r,fds_w;
    char c=0;
    FD_ZERO(&fds_r);
    FD_SET(0, &fds_r);
    int has_input = select(1, &fds_r, NULL, NULL, NULL);
    if(has_input){
        read(0,&c,1);
        char t;
        while(c != '\n' && read(0,&t,1) && t != '\n'&& t!= EOF);        //clear all input buffer
    }
    return c;
}

void sleep_ms(long ms){
    struct timespec tim;
    tim.tv_sec = 0;
    tim.tv_nsec = ms*1000000L;
    nanosleep(&tim,NULL);

}

void draw_pixel(int x, int y, color_t color){
    int increment = y*x_size + 2*x;
    *(color_t*)((char*)panel+increment) = color;
    sleep_ms(5);
}

void draw_rect(int x1, int y1, int width, int height, color_t c){
    int i,j;
    for(i=x1;i<x1+width;i++){
        for(j=y1;j<y1+height;j++){
            draw_pixel(i,j,c);
        }
    }
}

void draw_circle(int x0, int y0, int radius, color_t color){
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y)
    {
        draw_pixel(x0 + x, y0 + y,color);
        draw_pixel(x0 + y, y0 + x,color);
        draw_pixel(x0 - y, y0 + x,color);
        draw_pixel(x0 - x, y0 + y,color);
        draw_pixel(x0 - x, y0 - y,color);
        draw_pixel(x0 - y, y0 - x,color);
        draw_pixel(x0 + y, y0 - x,color);
        draw_pixel(x0 + x, y0 - y,color);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        else
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}