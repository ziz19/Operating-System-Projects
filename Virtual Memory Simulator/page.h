struct page_entry {
    int page_number;
    unsigned char valid;
    unsigned char dirty;
    unsigned char referenced;
    int virtual_time;    // an entire int for line number within 0 to 2,147,483,647;
};

typedef struct page_entry page;

int is_same_page(page p1,page p2);
page* INIT_PAGE_TABLE(int num_of_frames);