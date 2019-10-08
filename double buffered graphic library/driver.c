# include "library.h"
# include <fcntl.h>
# include <stdio.h>

int main() {
    clear_screen();
    init_graphics();
    char msg[] = "Please Enter Any Key to Continue:\n";
    write(1,msg,sizeof(msg)-1);
    char c = (char)getkey();
    draw_circle(320,240,60,2016);
    draw_rect(300,220,40,40,63488);
    int i;
    for(i = 220;i<420;i++)
        draw_pixel(i,240,31);
    char response[] = "You Pressed: ";
    write(1,response,sizeof(response)-1);
    write(1,&c,1);
    char new_line = '\n';
    write(1,&new_line,1);
    exit_graphics();
    return 0;
}
