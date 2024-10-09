#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct HEADER_TAG {
    struct HEADER_TAG * ptr_next;
    size_t bloc_size;
    long magic_number;
} HEADER;

void * malloc_3is(ssize_t size) {
    HEADER * new_block = sbrk(size + sizeof(HEADER) + sizeof(long));
    if (new_block == (void*)-1) {
        return NULL;
    }
    new_block->ptr_next = NULL;
    new_block->bloc_size = size;
    new_block->magic_number = 0x0123456789ABCDEFL;
    long * end_magic_number = (long *)(new_block+1)+size;
    *end_magic_number = 0x0123456789ABCDEFL;
    return new_block+1;
}

void free_3is(void * block) {


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

    return 0;
}
