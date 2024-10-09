# TP2
## Secure Memory Allocator

### Objectives
The goal of this lab is to develop a memory allocator to replace the standard `malloc`, `calloc`, and `free` functions. The standard glibc allocators are not very efficient, but they work well with both 1 byte and 1 GB.  
There are a series of alternative allocators that are highly effective for specific memory usage scenarios¹.  
You will create an allocator that will allow basic verification of memory corruption caused by buffer overflow in write operations.

### Allocator Implementation
You will test your memory allocator using a simple program that performs memory allocations and deallocations via calls to `malloc_3is` and `free_3is`.

Steps:
1. Multiple allocations


   When checking the correct allocation of each block (in the test), we compute the difference between the start address of the next block and the start address of the current block.  

   If this difference is equal to the size of the current block + the size of the end magic number + the size of the next block's header, then the allocation is correct.  
   Example: If we allocate a block of 100 bytes followed by a block of 200 bytes, the difference between the two addresses should be equal to 100 + 8 + 24 = 132 bytes.

Another thing to take into account is how the use of printf can change what we expected because it needs the heap to work. Basically, each time we used printf, we changed the address of the top of the heap.

2. Block deallocation



3. Checking for buffer overflows using magic numbers
4. Reusing a freed block (if the size is compatible)
5. Splitting large blocks if necessary
6. Merging adjacent free blocks
7. Pre-allocating memory
