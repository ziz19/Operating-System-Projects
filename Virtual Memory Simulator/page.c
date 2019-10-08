#include <stdlib.h>
#include "page.h"

int is_same_page(page p1, page p2){
    return p1.page_number == p2.page_number;
}

page* INIT_PAGE_TABLE(int num_of_frames){
    page* page_table = malloc(sizeof(page)*num_of_frames);
    int i;
    for(i=0;i<num_of_frames;i++){
        page_table[i].page_number = -1;
        page_table[i].dirty = 0;
        page_table[i].valid = 0;
        page_table[i].referenced = 0;
        page_table[i].virtual_time = -1;
    }
    return page_table;
}