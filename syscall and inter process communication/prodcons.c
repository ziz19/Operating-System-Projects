#include <stdio.h>
#include <linux/unistd.h>
#include <stdlib.h>
#include<sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>


struct cs1550_sem {
    int value;
    struct process_node* head;
    struct process_node* tail;
};

void down(struct cs1550_sem* sem) {
    syscall(__NR_sys_cs1550_down, sem);
}

void up(struct cs1550_sem* sem){
    syscall(__NR_sys_cs1550_up, sem);
}

int main(int argc, char* argv[]){
    int num_producers = atoi(argv[1]);
    int num_customers = atoi(argv[2]);
    int n = atoi(argv[3]);  //buffer size

    struct cs1550_sem* ptr = (struct cs1550_sem*)mmap(NULL, sizeof(struct cs1550_sem)*3, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    struct cs1550_sem* empty = ptr;
    empty->value = n;
    empty->head = NULL;
    empty->tail = NULL;
    struct cs1550_sem* full = ptr+1;
    full->value = 0;
    full->head = NULL;
    full->head = NULL;
    struct cs1550_sem* mutex = ptr+2;
    mutex->value = 1;
    mutex->head = NULL;
    mutex->tail = NULL;

    int* item_ptr = (int*) mmap(NULL, (n+2)*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    int* in = item_ptr;
    int* out = item_ptr+1;
    int* buffer = item_ptr+2;

    int i;
    for(i = 0; i < num_producers; i++){
        if(fork() == 0){
            while(1){
                down(empty);
                down(mutex);
                buffer[*in] = *in;
                printf("Chef %c Produced: Pancake%d\n", (i+65), *in);
                *in = (*in+1)%n;
                up(mutex);
                up(full);
            }
        }
    }

    for(i = 0; i < num_customers; i++){
        if(fork() == 0){
            while(1){
                down(full);
                down(mutex);
                int item = buffer[*out];
                printf("Customer %c Consumed: Pancake%d\n", (i+65), item);
                *out = (*out+1)%n;
                up(mutex);
                up(empty);
            }
        }
    }

    int status;
    wait(&status);

    return 0;
}