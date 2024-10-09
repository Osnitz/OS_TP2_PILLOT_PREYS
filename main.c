#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAGIC_NUMBER 0x0123456789ABCDEFL

typedef struct HEADER_TAG {
    struct HEADER_TAG * ptr_next;
    size_t bloc_size;
    long magic_number;
} HEADER;

HEADER * header_free_list = NULL;

HEADER* find_block_of_size(ssize_t size) {
    HEADER* current = header_free_list;
    HEADER* found = NULL;
    while (current != NULL) {
        if (current->ptr_next->bloc_size == size) {
            //printf("existant block found\n");
            found = current->ptr_next;
            current->ptr_next = current->ptr_next->ptr_next;
            return found;
        }
        current = current->ptr_next;
    }
    return NULL;
}
void * malloc_3is(ssize_t size) {
    HEADER * new_block = find_block_of_size(size);
    if (new_block == NULL) {
        new_block = sbrk(size + sizeof(HEADER) + sizeof(long));
        if (new_block == (void*)-1) {
            return NULL;
        }
        new_block->ptr_next = NULL;
        new_block->bloc_size = size;
        new_block->magic_number = MAGIC_NUMBER;
        long * end_magic_number = (long *)(new_block+1)+size;
        *end_magic_number = MAGIC_NUMBER;
    }

    return new_block+1;
}

void displayError() {
    printf("Memory overflow detected\n");
}

void free_3is(void * block) {
    if (block == NULL) {
        return;
    }
    HEADER * block_to_free = block - sizeof(HEADER);
    ssize_t block_size = block_to_free->bloc_size;
    long * end_magic_number = (long *)block+block_size;

    if ((block_to_free->magic_number != MAGIC_NUMBER) || (*end_magic_number != MAGIC_NUMBER)) {
        displayError();
        return;
    }
    block_to_free->ptr_next = header_free_list;
    header_free_list = block_to_free;
}

void print_linked_list() {
    if (header_free_list == NULL) {
        printf("NULL\n");
        return;
    }
    HEADER* current = header_free_list;
    printf("<%p>, %lu -> ", current, current->bloc_size);
    current = current->ptr_next;
    while (current != NULL) {
        printf("<%p>, %lu -> ", current, current->bloc_size);
        current = current->ptr_next;
    };
    printf("NULL\n");
}



int main(void) {
    void * ptr = sbrk(0);
    printf("Base memory : %p\n", ptr);

    void * block1 = malloc_3is(100);
    printf("block1: %p\n", block1);
    printf("Increased by : %ld\n", block1 - ptr);


    void * block2 = malloc_3is(200);
    printf("block2: %p\n", block2);
    printf("Increased by (expected = 132) : %ld\n", block2 - block1);

    void * block3 = malloc_3is(300);
    printf("block3: %p\n", block3);
    printf("Increased by (expected = 232): %ld\n", block3 - block2);

    free_3is(block1);
    print_linked_list();
    free_3is(block2);
    print_linked_list();

    void * block4 = malloc_3is(100);
    printf("block4: %p\n", block4);
    print_linked_list();

    return 0;
}
