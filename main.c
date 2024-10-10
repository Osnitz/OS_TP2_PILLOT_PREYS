//
// Created by matthieu on 09/10/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define MAGIC_NUMBER 0x0123456789ABCDEFL

typedef struct HEADER_TAG {
    struct HEADER_TAG * ptr_next;
    size_t bloc_size;
    long magic_number;
} HEADER;

HEADER * header_free_list = NULL;

bool are_blocks_adjacent(void* block1_void, void* block2_void) {
    if (block1_void == NULL || block2_void == NULL) {
        return false;
    }
    HEADER* block1 = (HEADER*)block1_void - 1;
    HEADER* block2 = (HEADER*)block2_void - 1;
    void* end_of_block1 = (void*)(block1 + 1) + block1->bloc_size + sizeof(long);
    return end_of_block1 == (void*)block2;
}

void insert_and_merge_free_blocks(HEADER* block) {
    HEADER* current = header_free_list;

    if (current == NULL) {
        header_free_list = block;
        block->ptr_next = NULL;
        return;
    }

    while (current->ptr_next != NULL && current->ptr_next < block) {
        current = current->ptr_next;
    }

    if (current->ptr_next != NULL && are_blocks_adjacent(block + 1, current->ptr_next + 1)) {
        block->bloc_size += sizeof(HEADER) + current->ptr_next->bloc_size + sizeof(long);
        block->ptr_next = current->ptr_next->ptr_next;
    } else {
        block->ptr_next = current->ptr_next;
    }

    if (are_blocks_adjacent(current + 1, block + 1)) {
        current->bloc_size += sizeof(HEADER) + block->bloc_size + sizeof(long);
        current->ptr_next = block->ptr_next;
    } else {
        current->ptr_next = block;
    }
}

void split_block(HEADER* block, ssize_t wanted_size) {
    HEADER * first_block = block;
    HEADER* second_block= (void*)(block + 1) + wanted_size + sizeof(long);
    second_block->ptr_next = first_block->ptr_next;
    second_block->bloc_size = first_block->bloc_size - wanted_size - sizeof(HEADER)-sizeof(long);
    second_block->magic_number = MAGIC_NUMBER;
    long* second_end_magic_number = (long*)(second_block+1)+second_block->bloc_size;
    *second_end_magic_number = MAGIC_NUMBER;

    first_block->bloc_size = wanted_size;
    first_block->ptr_next = second_block;
    long* first_end_magic_number = (long*)(first_block+1)+first_block->bloc_size;
    *first_end_magic_number = MAGIC_NUMBER;
}

HEADER* find_block_of_size(ssize_t size)
{
    HEADER* current = header_free_list;
    HEADER* found = NULL;

    if (current != NULL && current->bloc_size >= size){
        if (current->bloc_size > size){
            split_block(current, size);
        }
        if (current->bloc_size == size){
            found = current;
            header_free_list = current->ptr_next;
            return found;
        }
    }
    while (current != NULL){
        if (current->ptr_next->bloc_size >= size){
            //printf("existant block found\n");
            if (current->ptr_next->bloc_size > size){
                split_block(current->ptr_next, size);
            }
            if (current->ptr_next->bloc_size == size){
                found = current->ptr_next;
                current->ptr_next = current->ptr_next->ptr_next;
                return found;
            }
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

    insert_and_merge_free_blocks(block_to_free);
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
    /*printf("--------------- Allocation of 3 blocks ---------------\n");
    void * ptr = sbrk(0);
    printf("Base memory : %p\n", ptr);

    void * block1 = malloc_3is(100);
    printf("\nblock1: %p\n", block1);
    printf("Increased by : %ld\n", block1 - ptr);


    void * block2 = malloc_3is(50);
    printf("\nblock2: %p\n", block2);
    printf("Increased by (expected = 132) : %ld\n", block2 - block1);

    void * block3 = malloc_3is(300);
    printf("\nblock3: %p\n", block3);
    printf("Increased by (expected = 232): %ld\n", block3 - block2);

    printf("\n--------------- Free block 1, 2, 3 and clear free list ---------------\n");
    free_3is(block1);
    print_linked_list();
    free_3is(block2);
    print_linked_list();
    free_3is(block3);
    print_linked_list();
    header_free_list = NULL;


    printf("\n--------------- Spliting blocks ---------------\n");
    void * block4 = malloc_3is(200);
    printf("\nblock4: %p\n", block4);
    HEADER * full_block4 = block4 - sizeof(HEADER);
    free_3is(block4);
    print_linked_list();
    split_block(full_block4, 100);
    print_linked_list();

    printf("\n--------------- Use freed blocks ---------------\n");
    print_linked_list();
    void* block5 = malloc_3is(60);
    printf("block5: %p\n", block5);
    print_linked_list();

    void* block6 = malloc_3is(50);
    printf("block6: %p\n", block6);
    print_linked_list();
    header_free_list = NULL;*/

    printf("\n--------------- Fusion of 2 blocks ---------------\n");
    void* block7 = malloc_3is(100);
    void* block8 = malloc_3is(50);
    printf("block7: %p\n", block7);
    printf("block8: %p\n", block8);
    void*bloc9 = malloc_3is(60);
    printf("bloc9: %p\n", bloc9);
    void* bloc10 = malloc_3is(70);
    printf("bloc10: %p\n", bloc10);

    /*if(are_blocks_adjacent(block7, block8))
    {
        printf("block7 and block8 are adjacent\n");
    }
    else{
        printf("block7 and block8 are not adjacent\n");
    }*/

    free_3is(block7);
    print_linked_list();
    free_3is(bloc9);
    print_linked_list(); //expected output: <block7>, 132 -> <bloc9>, 92 -> NULL
    free_3is(bloc10);
    print_linked_list();//<block7>, 132 -> <bloc9 + bloc10>, 194 -> NULL
    free_3is(block8);
    print_linked_list();//<block 7 + 8 + 9 + 10>, 408 -> NULL

    return 0;
}
