#include <stdio.h>

void execute_wsclock();   // execute the clock algorithm
void init_wsclock(FILE* fd, int num_of_frames, int t);  // initialize with the trace file, number of frames, tau
void print_wsclock_stats();
void exit_wsclock();