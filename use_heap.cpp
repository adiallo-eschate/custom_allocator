#include "heap_init.cpp"
#include <exception>
#include <iostream>


int main(){

    try{
        size_t size = 4000;
        user_heap_init();
        void* ptr = user_allocate(size);

        int* num_ptr = static_cast<int*>(ptr);

        *num_ptr = 2000000;

        std::cout << "Hello World! It Worked " << *num_ptr << std::endl;

        user_free(ptr);
        
        std::cout << "Block Returned to Memory. Hooray!" << std::endl;

        char* char_ptr = (char*)user_allocate(200);

        *char_ptr = 'c';
        std::cout << "new_ptr: " << char_ptr << std::endl;

        user_free(char_ptr);

    } catch(const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    
    return 0;
}