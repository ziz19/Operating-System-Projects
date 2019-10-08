#include <stdio.h>
#include <limits.h>
#include "uthash.h"
#include "opt.h"
#include "page.h"
#include "queue.h"

/************      helper structure for storing knowledge *************/
struct page_info{
    int id;
    struct Queue* recent_access;
    UT_hash_handle hh;
};

static struct page_info* page_trace = NULL;
static int entry_count = 0;


static void update_page(int page_number, int access_time){
    struct page_info* s;
    HASH_FIND_INT(page_trace,&page_number,s);   // find if page is already inside
    if (s == NULL){              // if not, add the page into the hash table
        s = (struct page_info*)malloc(sizeof(struct page_info));
        s->id = page_number;
        s->recent_access = createQueue(500000);
        enqueue(s->recent_access,access_time);
        HASH_ADD_INT(page_trace,id,s);
        entry_count ++;
    }else{
        struct Queue* next_occur = s->recent_access;
        enqueue(next_occur,access_time);
    }
}

static int find_next_reference(int page_number, int access_time){
    int next = INT_MAX;  // next referenced time
    struct page_info* s;
    HASH_FIND_INT(page_trace,&page_number,s);
    if (s == NULL){
        return next;
    }
    next = front(s->recent_access);     // peek at the next access
    return next;
}

static void update_reference(int page_number, int access_time){
    struct page_info* s;
    HASH_FIND_INT(page_trace,&page_number,s);
    if (s == NULL){
//        printf("Page not in the hash table\n");
        return;
    }
    if(front(s->recent_access) != access_time){
        printf("Page info inconsistent  "
                       "Page number: %x  "
                       "Next access: %d  "
                       "Access time: %d\n",page_number,front(s->recent_access),access_time);
        return;
    }
    dequeue(s->recent_access);
    if(isEmpty(s->recent_access)){
        HASH_DEL(page_trace,s);
        free(s->recent_access);
        free(s);
    }
}

static void free_page_trace(){
    struct page_info *current_page, *tmp;
    HASH_ITER(hh, page_trace, current_page, tmp) {
        struct Queue* temp = current_page->recent_access;
        while(!isEmpty(temp)){
            dequeue(temp);
        }
        free(current_page);             /* free it */
    }
}

/********** OPT algorithm starts here   *************************/
static FILE* fp;
static int total_frames = 0;
static int mem_access = 0;     // counts for memory access
static int page_faults = 0;    // counts for page faults
static int disk_writes = 0;    // counts for disk writes
static page* page_table;

void init_opt(FILE* fd, int num_of_frames,FILE* copy){
    total_frames = num_of_frames;
    fp = fd;
    page_table = INIT_PAGE_TABLE(num_of_frames);
    unsigned int address;       /* preprocess the trace file */
    char mode;
    int line = 1;
    printf("Preprocess the trace file...\n");
    while(fscanf(copy,"%x %c", &address, &mode) != EOF){
        update_page(address>>12,line++);
//        printf("%d\n",line++);
    }
    fclose(copy);
    printf("Unique pages: %d\n",entry_count);
}

void print_opt_stats(){
    printf("OPT\n"
                   "Number of frames: %d\n"
                   "Total memory accesses: %d\n"
                   "Total page faults: %d\n"
                   "Total writes to disk: %d\n",total_frames,mem_access,page_faults,disk_writes);
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

static int find_farthest_page(){
    int farthest = INT_MIN;
    int evict = -1;
    int i;
    for(i=0;i<total_frames;i++){
        int occured = find_next_reference(page_table[i].page_number,page_table[i].virtual_time);
        if (occured > farthest){
            evict = i;
            farthest = occured;
        };
//        printf("%d ith %x page will be referenced at %d\n",i,page_table[i].page_number,occured);
    }
    return evict;
}

void execute_opt(){
    int available_frames = total_frames;
    int next = 0;   // next location in the page table
    unsigned int address;       /* preprocess the trace file */
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
        else{                   // evict a page out of table
            int evict = find_farthest_page();
            if(page_table[evict].dirty == 1){
                disk_writes++;
                printf("page fault-evict dirty\n");
            }else{
                printf("page fault-evict clean\n");
            }
            page_table[evict] = new_access;
            page_faults++;
        }
        update_reference(new_access.page_number,new_access.virtual_time);   // update the page info
        mem_access++;
//        print_table();
        }
}


void exit_opt(){
    fclose(fp);
    free_page_trace();
    free(page_table);

}
