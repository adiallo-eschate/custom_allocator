# CUSTOM MEMORY ALLOCATOR

A lightweigh allocator written in C++ to learn about low-level memory management and systems programming. The allocator manages a contiguous memory region using a free list which is a linked list (singly).

## Features

- Free list management with block splitting and coalescing
- Block metadata headers for size and allocation status
- Fixed-size heap with configurable size
- `user_allocate()` and `user_free()` functions for the user

## usage

'''c++
void* ptr = user_allocate(size);
user_free(ptr);


## What I learned 

- Use print statements generously
- Memory layout  
- pointer arithmetic
- Designing robust low-level APIs


