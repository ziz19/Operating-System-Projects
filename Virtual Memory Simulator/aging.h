#include <stdio.h>

void execute_aging();   // execute the aging algorithm
void init_aging(FILE* fd, int num_of_frames,int refresh);  // initialize with trace file, maximum number of frames, refresh period
void print_aging_stats();
void exit_aging();
