#include <stdlib.h>
#include <limits.h>
#include "wsclock.h"
#include "page.h"

static FILE* fp;
static int total_frames = 0;
static int tau = 0;
static int mem_access = 0;     // counts for memory access
static int page_faults = 0;    // counts for page faults
static int disk_writes = 0;    // counts for disk writes
static page* page_table;


void init_wsclock(FILE* fd,int num_of_frames,int t){
    total_frames = num_of_frames;
    fp = fd;
    page_table = INIT_PAGE_TABLE(num_of_frames);
    tau = t;
}

void print_wsclock_stats(){
    printf("WSClock\n"
                   "Number of frames: %d\n"
                   "Tau: %d\n"
                   "Total memory accesses: %d\n"
                   "Total page faults: %d\n"
                   "Total writes to disk: %d\n",total_frames,tau,mem_access,page_faults,disk_writes);

}

static void print_table(){
    int i;
    for(i=0;i<total_frames;i++){
        printf("Slot %d Page Number %08x Dirty %d Valid %d referenced %d virtual_time %d\n",
               i,page_table[i].page_number,page_table[i].dirty,page_table[i].valid,page_table[i].referenced,page_table[i].virtual_time);
    }
    printf("*******************************\n");
}

static int already_in_table(page new_access){
    int exist = -1;
    int i;
    for(i=0;i<total_frames;i++){
        if(is_same_page(page_table[i],new_access))  return i;      // if the page is in the table, return its location
    }
    return exist;
}

void execute_wsclock(){
    int available_frames = total_frames;
    int next = 0;   // next location in the page table
    int page_to_examine = 0;  // next page that will be examined
    unsigned int address;
    char mode;
    while(fscanf(fp,"%x %c", &address, &mode) != EOF){
//        printf("%08x,%c\n",address,mode);
        page new_access = {address>>12,1,mode=='W',1,mem_access+1};   // create a page acess request
        /* add the frame into the frame array */
        int location_in_table = already_in_table(new_access);
        if( location_in_table != -1){       // if page is already in table, update stats
            page_table[location_in_table].referenced = 1;
            page_table[location_in_table].virtual_time = mem_access+1;
            if(new_access.dirty) page_table[location_in_table].dirty = 1;
            printf("hit\n");
        }
        else if(available_frames){           // if there are spaces available, add directly
            page_table[next] = new_access;
            available_frames--;
            next = (next+1)%total_frames;
            page_faults++;
            printf("page fault-no eviction\n");
        }
        else{                             // evict page out of table
            /* find the page to evict */
            int count = 0;
            int evict = -1; // page to be evicted;
            int oldest_time = INT_MAX;  // oldest timestamp
            while(count < total_frames){      // if haven't found a page or haven't traversed entire table
                if (page_table[page_to_examine].referenced == 1){       /* if a page is referenced */
                    page_table[page_to_examine].referenced = 0;
                    page_table[page_to_examine].virtual_time = mem_access+1;    // update time to be current line number
                }
                else{
                    if (page_table[page_to_examine].dirty){         // if a page is unreferenced and dirty
                        disk_writes++;
                        page_table[page_to_examine].dirty = 0;
                    } else if(mem_access+1 - page_table[page_to_examine].virtual_time > tau){   // if a page is unreferenced, clean and old
                        evict = page_to_examine;    // an ideal evict page is found
                        page_to_examine = (page_to_examine+1)%total_frames;
                        break;
                    }
                }
                if (page_table[page_to_examine].virtual_time < oldest_time){
                    oldest_time = page_table[page_to_examine].virtual_time;
                    evict = page_to_examine;
                }
                page_to_examine = (page_to_examine+1)%total_frames;
                count++;
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

void exit_wsclock(){
    fclose(fp);
    free(page_table);
}

