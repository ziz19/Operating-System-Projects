#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "page.h"
#include "aging.h"

static FILE* fp;
static int total_frames = 0;
static int mem_access = 0;     // counts for memory access
static int page_faults = 0;    // counts for page faults
static int disk_writes = 0;    // counts for disk writes
static int period = 0;     // refresh period for aging
static page* page_table;

void init_aging(FILE* fd, int num_of_frames,int refresh){
    total_frames = num_of_frames;
    fp = fd;
    page_table = INIT_PAGE_TABLE(num_of_frames);
    period = refresh;
}

void print_aging_stats(){
    printf("Aging\n"
                   "Number of frames: %d\n"
                   "Refresh time: %d\n"
                   "Total memory accesses: %d\n"
                   "Total page faults: %d\n"
                   "Total writes to disk: %d\n",total_frames,period,mem_access,page_faults,disk_writes);

}

static int already_in_table(page new_access){
    int exist = -1;
    int i;
    for(i=0;i<total_frames;i++){
        if(is_same_page(page_table[i],new_access))  return i;      // if the page is in the table, return its location
    }
    return exist;
}

static void print_table(){
    int i;
    for(i=0;i<total_frames;i++){
        printf("Slot %d Page Number %08x Dirty %d Valid %d referenced %d virtual_time %d\n",
               i,page_table[i].page_number,page_table[i].dirty,page_table[i].valid,page_table[i].referenced,page_table[i].virtual_time);
    }
    printf("*******************************\n");
}

static void shift_bits(){
    int i;
    for(i=0;i<total_frames;i++){
        page_table[i].referenced = page_table[i].referenced>>1;
    }
}

void execute_aging(){
    int available_frames = total_frames;
    int next = 0;   // next location in the page table
    unsigned int address;
    char mode;
    while(fscanf(fp,"%x %c", &address, &mode) != EOF){
        /* shift all reference bits periodically */
        if(mem_access % period == 0)
            shift_bits();
//        printf("%08x,%c\n",address,mode);
        page new_access = {address>>12,1,mode=='W',128,mem_access+1};
        /* add the frame into the frame array */
        int location_in_table = already_in_table(new_access);
        if (location_in_table != -1){       // if page is already in table, update stats
            page_table[location_in_table].referenced |= 128;
            if(new_access.dirty) page_table[location_in_table].dirty = 1;
            printf("hit\n");
        }
        else if(available_frames){         // if there are spaces available, add directly
            page_table[next] = new_access;
            available_frames--;
            next++;
            page_faults++;
            printf("page fault-no eviction\n");
        }
        else{                             // evict page out of table
            /* find the page to evict */
            int evict = -1;    // the index of the page to be evict
            int smallest_referenced = INT_MAX;  // the smallest reference bits
            int i;
            for(i=0;i<total_frames;i++){
                if (page_table[i].referenced < smallest_referenced){
                    evict = i;
                    smallest_referenced = page_table[i].referenced;
                }
            }
            /* evict the page */
            if(page_table[evict].dirty == 1){
                disk_writes++;
                printf("page fault-evict dirty\n");
            }else{
                printf("page fault-evict clean\n");
            }
            page_table[evict] = new_access;
            page_faults++;
        }
        mem_access++;
//        print_table();
    }
}

void exit_aging(){
    fclose(fp);
    free(page_table);
}