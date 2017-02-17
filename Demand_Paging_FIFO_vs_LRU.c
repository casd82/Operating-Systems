#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PAGE_RANGE 9
#define PAGE_NUM 7
#define DEMANDS 1000

typedef struct _node {
    int val;
    struct _node * prev;
    struct _node * next;
} node;

int rand_page_num() {
    return rand() % (PAGE_RANGE + 1);
}

/* sequential search, return the node if found */
node * search(node * start, int target) {
    start = start->next;

    while (start->next) {
        if (target == start->val) {
            return start;
        }

        start = start->next;
    }

    return NULL;
}

void print(node * start) {
    start = start->next;

    printf("[ ");
    while (start->next) {
        printf("%d ", start->val);
        start = start->next;
    }
    printf("]\n");
}

void free_all(node * start) {
    node * temp = start;
    while (temp) {
        start = start->next;
        free(temp);
        temp = start;
    }
}


/* queue */
void enqueue(node * start, int val) {
    node * new_node = (node *) malloc(sizeof(node));
    new_node->val = val;
    new_node->next = start->next;
    start->next->prev = new_node;
    new_node->prev = start;
    start->next = new_node;
}

void dequeue(node * end) {
    node * temp = end->prev;
    end->prev->prev->next = end;
    end->prev = end->prev->prev;
    free(temp);
}

/* stack */
void move_to_top(node * top, node * target) {
    target->prev->next = target->next;
    target->next->prev = target->prev;
    top->next->prev = target;
    target->next = top->next;
    target->prev = top;
    top->next = target;
}

/* return page fault counts */
int FIFO() {
    int faults = 0, i, curr_size = 0;

    /* initializing queue */
    node * q_head = (node *) malloc(sizeof(node));
    node * q_end = (node *) malloc(sizeof(node));
    q_head->next = q_end;
    q_end->prev = q_head;
    q_head->prev = q_end->next = NULL;

    for (i = 0; i < DEMANDS; i++) {

        /* get a new random page number */
        int curr_num = rand_page_num();

        if (!search(q_head, curr_num)) {
            faults++;
            printf("Accessing number %d, but page fault occurs!\n", curr_num);
            
            if (curr_size >= PAGE_RANGE) { /* no free pages left */
                dequeue(q_end);
                curr_size--;
                printf("Dequeue...\n");
            }

            enqueue(q_head, curr_num);
            curr_size++;
            printf("Enqueue %d\n", curr_num);
        } else {
            printf("Accessing number %d...\n", curr_num);
        }

        printf("Memory Now: ");
        print(q_head);

    }

    free_all(q_head);
    return faults;
}

/* return page fault counts */
int LRU() {
    int faults = 0, i, curr_size = 0;

    /* init stack */
    node * s_bottom = (node *) malloc(sizeof(node));
    node * s_top = (node *) malloc(sizeof(node));
    s_top->next = s_bottom;
    s_bottom->prev = s_top;
    s_top->prev = s_bottom->next = NULL;

    for (i = 0; i < DEMANDS; i++) {
        int curr_num = rand_page_num();

        node * target = search(s_top, curr_num);
        if (target) {
            /* target found, move to top */
            printf("Accessing number %d, move em to the top...\n", curr_num);
            move_to_top(s_top, target);
        } else {
            printf("Accessing number %d, but page fault occurs!\n", curr_num);
            faults++;
            if (curr_size >= PAGE_RANGE) { /* no free pages left */
                printf("Remove the least recent used...\n");
                curr_size--;
                dequeue(s_bottom);
            }

            printf("Stack %d to the top...\n", curr_num);
            curr_size++;
            enqueue(s_top, curr_num);
        }

        printf("Memory Now: ");
        print(s_top);
    }

    free_all(s_top);

    return faults;
}

int main(void) {
    srand(time(NULL));
    int faults1, faults2;

    printf("========FIFO=========\n");
    faults1 = FIFO();
    printf("======Page Faults: %d========\n", faults1);

    printf("\n==========LRU=========\n");
    faults2 = LRU();
    printf("=========Page Faults: %d========\n", faults2);

    printf("DONE! FIFO faults = %d, LRU faults = %d\n", faults1, faults2);

    return 0;
}