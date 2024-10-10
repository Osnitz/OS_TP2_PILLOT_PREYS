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
    if(current !=NULL && current->bloc_size == size) {
        found = current;
        header_free_list = current->ptr_next;
        found->ptr_next = NULL;
        return found;
    }
    while (current != NULL) {
        if (current->ptr_next!=NULL && current->ptr_next->bloc_size == size) {
            //printf("existant block found\n");
            found = current->ptr_next;
            current->ptr_next = current->ptr_next->ptr_next;
            found->ptr_next = NULL;
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
    printf("<%p>, %lu -> ", current, current->bloc_size+sizeof(HEADER)+sizeof(long));
    current = current->ptr_next;
    while (current != NULL) {
        printf("<%p>, %lu -> ", current, current->bloc_size+sizeof(HEADER)+sizeof(long));
        current = current->ptr_next;
    };
    printf("NULL\n");
}



int main(void) {

    printf("--------------- Allocation of 3 blocks ---------------\n");
    void * ptr = sbrk(0);
    printf("Base memory : %p\n", ptr);

    void * block1 = malloc_3is(100);
    printf("\nblock1: %p, size = %lu \n", block1, ((HEADER*)block1-1)->bloc_size);

    void * block2 = malloc_3is(50);
    printf("\nblock2: %p, size = %lu \n", block2, ((HEADER*)block2-1)->bloc_size);

    void * block3 = malloc_3is(60);
    printf("\nblock3: %p,  size = %lu \n", block3, ((HEADER*)block3-1)->bloc_size);

    printf("\n--------------- Free block 1, 2, 3  ---------------\n");
    printf("Free block1\n");
    free_3is(block1);
    print_linked_list();
    printf("Free block2\n");
    free_3is(block2);
    print_linked_list();
    printf("Free block3\n");
    free_3is(block3);
    print_linked_list();

    printf("\n--------------- Use of freed blocks ---------------\n");
    printf("Allocation of block4 of size 60\n");
    void* block4 = malloc_3is(60);
    print_linked_list();
    printf("Allocation of block5 of size 50\n");
    void* block5 = malloc_3is(50);
    print_linked_list();

    return 0;
}