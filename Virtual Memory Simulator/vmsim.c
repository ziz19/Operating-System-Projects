#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "clock.h"
#include "aging.h"
#include "wsclock.h"
#include "opt.h"

int main(int argc, char** argv){
    if (argc < 6 ){
        printf("You need to pass enough arguments");
        return -1;
    }
    int i=1;
    int num_of_frames=0;
    char* algorithm="";
    int refresh=0;
    int tau=0;
    char* file_name="";
    while(i < argc){
        if(strcmp(argv[i],"-n")==0)    num_of_frames = atoi(argv[i+1]);
        else if(strcmp(argv[i],"-a")==0) algorithm = argv[i+1];
        else if(strcmp(argv[i],"-r")==0) refresh = atoi(argv[i+1]);
        else if(strcmp(argv[i],"-t")==0) tau = atoi(argv[i+1]);
        else if(i == argc -1 ) file_name = argv[i];
        else{
            printf("Invalid arguments\n");
            return -2;
        }
        i += 2;
    }
    /* initialize mode and variables */
    FILE* fp = fopen(file_name,"r");
    FILE* fp2 = fopen(file_name,"r");

    /* run corresponding algorithm */
    if (strcmp(algorithm,"clock") == 0){
        init_clock(fp,num_of_frames);
        execute_clock();
        print_clock_stats();
        exit_clock();
    }else if(strcmp(algorithm,"aging") == 0){
        init_aging(fp,num_of_frames,refresh);
        execute_aging();
        print_aging_stats();
        exit_aging();
    }else if(strcmp(algorithm,"work") == 0){
        init_wsclock(fp,num_of_frames,tau);
        execute_wsclock();
        print_wsclock_stats();
        exit_wsclock();
    }else if(strcmp(algorithm,"opt") == 0){
        init_opt(fp,num_of_frames,fp2);
        execute_opt();
        print_opt_stats();
        exit_opt();
    }
    return 0;
}