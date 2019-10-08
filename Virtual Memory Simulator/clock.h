#include <stdio.h>

void execute_clock();   // execute the clock algorithm
void init_clock(FILE* fd, int num_of_frames);  // initialize the clock with the trace file, required number of frames
void print_clock_stats();
void exit_clock();