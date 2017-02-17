#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#define PAGE_SIZE 256
#define PAGE_TABLE_SIZE 256
#define FRAME_SIZE 256
#define FRAME_ENTRIES 256
#define TLB_SIZE 16
#define MEM_SIZE (FRAME_SIZE * FRAME_ENTRIES)
#define LINE_SIZE 8
#define PAGE_BITS 8
#define OFFSET_BITS 8
#define TLB_MASK 0x00ff

#define OFFSET(x)        (x & 0x00ff)
#define PAGE_NUM(x)      (x >> OFFSET_BITS)
#define PHYSICAL(x, y)   ((x << OFFSET_BITS) + y)

int page_table[PAGE_TABLE_SIZE] = {0}; /*frame number (8 bit) + (1 valid bit) */
int TLB[TLB_SIZE + 1] = {0}; /* page number (8 bit) -> frame number (8 bit) + (1 valid bit) */
char memory[MEM_SIZE];
int mem_index = 0;

/* loop array as a queue */
int tlb_begin = 0;
int tlb_end = 0;

/* FIFO scheme */
void update_TLB(int page_number, int frame_number) {
    /* check if tlb is full */
    if ((tlb_end + 1) % TLB_SIZE == tlb_begin) {
        /* dequeue */
        tlb_begin = (tlb_begin + 1) % TLB_SIZE;
    }

    /* enqueue */
    TLB[tlb_end] = (page_number << OFFSET_BITS << 1) + (frame_number << 1) + 1;

    tlb_end = (tlb_end + 1) % TLB_SIZE;
}

/* return 1 for success, 0 for failure */
int check_TLB(int page_number, int *frame_number) {
    int i;
    for (i = 0; i < TLB_SIZE; i++) {
        /* is valid and have the correct address */
        if ((TLB[i] & 1) && (page_number == (TLB[i] >> (OFFSET_BITS + 1)))) {
            *frame_number = (TLB[i] >> 1) & TLB_MASK;
            return 1;
        }
    }
    return 0;
}

/* return 1 for success, 0 for failure */
int check_page_table(int page_number, int *frame_number) {
    if (page_table[page_number] & 1) {
        *frame_number = page_table[page_number] >> 1;
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Parameters are: Input File, Output File, Backing Store\n");
        return 1;
    }

    /* files */
    char *in_file = argv[1];
    char *out_file = argv[2];
    char *store_file = argv[3];
    char *store_data;

    int store_fd;

    FILE *in_ptr;
    FILE *out_ptr;

    /* address file */
    if ((in_ptr = fopen(in_file, "r")) == NULL) {
        printf("Cannot open file %s\n", in_file);
        return 2;
    }

    /* output file */
    if ((out_ptr = fopen(out_file, "a")) == NULL) {
        printf("Cannot open file %s\n", out_file);
        return 3;
    }

    /* store file */
    store_fd = open(store_file, O_RDONLY);
    store_data = mmap(0, MEM_SIZE, PROT_READ, MAP_SHARED, store_fd, 0);
    if (store_data == MAP_FAILED) {
        printf("Mapping %s failed\n", store_file);
        close(store_fd);
        return 4;
    }
    char line[LINE_SIZE];
    /* start reading from the address list */
    while (fgets(line, sizeof(line), in_ptr)) {
        int virtual = atoi(line);
        int frame_number;
        int page_number = PAGE_NUM(virtual);
        int offset = OFFSET(virtual);
        int physical;

        /* first look at TLB & page table*/
        if (check_TLB(page_number, &frame_number) || check_page_table(page_number, &frame_number)) {
            physical = PHYSICAL(frame_number, offset);
            fprintf(out_ptr, "Virtual Address: %d, Physical Address: %d, value: %d\n", virtual, physical, memory[physical]);
            printf("Virtual Address: %d, Physical Address: %d, value: %d\n", virtual, physical, memory[physical]);
        } else {
            /* fault */
            int page_address = page_number * PAGE_SIZE;

            if (mem_index != -1) { /* if a free frame exists */
                
                /* copy the page from store file to memory frame */
                memcpy(memory + mem_index, store_data + page_address, PAGE_SIZE);

                /* calculate the physical address */
                frame_number = mem_index;
                physical = frame_number + offset;

                /* update page table (set valid bit to 1) */
                page_table[page_number] = (frame_number >> (OFFSET_BITS - 1)) + 1;
                

                /* update TLB */
                update_TLB(page_number, frame_number >> OFFSET_BITS);

                fprintf(out_ptr, "Virtual address: %d, Physical address: %d, Value: %d\n", virtual, physical, memory[physical]);
                printf("Virtual Address: %d, Physical Address: %d, value: %d\n", virtual, physical, memory[physical]);
                /* update mem_index */
                if (mem_index < MEM_SIZE - FRAME_SIZE) {
                    mem_index += FRAME_SIZE;
                } else {
                    mem_index = -1; /* memory full */
                }
            }
        }

    }

    fclose(in_ptr);
    fclose(out_ptr);
    close(store_fd);

    return 0;
}