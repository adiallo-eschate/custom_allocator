#include <iostream>
#include <memoryapi.h>
#include <exception>
#include <assert.h>


using namespace std;

struct Node {
    size_t size{};       // length of entire node including Node info
    Node* memory_location{};
    Node* next{nullptr};
};

struct Header {
    size_t size{};   // does not include the header itself
   // void* user_ptr{nullptr};
};


struct Linked_List {

    Linked_List(){ 
            cout << "Linked List Object Initialized But Empty" << endl;
    }

    void init_head(Node *head_node_ptr, size_t size){
        head = head_node_ptr;
        head->size = size;
        head->memory_location = head_node_ptr;
        head->next = nullptr;
    }

    void print_head_ptr(){
        cout << "Current List Head Pointer is: "<< head << endl;
    }

    Node* get_head_ptr(){
        return head;
    }

    void set_head_ptr(Node* new_head){
        head = new_head;
        cout << "Head Pointer Updated Sucessfully" << endl;
    }


    void add_to_front(Node *new_node){
        
    }

    private:
    Node* head{};
};


struct Heap {

    Heap(){
        memory_head_ptr = VirtualAlloc(NULL, heap_size, MEM_COMMIT| MEM_RESERVE, PAGE_READWRITE);
        cout << "Memory begins at " << memory_head_ptr << endl;
    }

    ~Heap(){
        VirtualFree(memory_head_ptr, 0, MEM_RELEASE);
        cout << "Heap Destroyed" << endl;
    }

    Heap (const Heap& other) = delete;
    Heap& operator=(Heap& other) = delete;

    Heap (Heap&& other) = delete;
    Heap& operator=(Heap&& other) = delete;


    void heap_init(){
        // copy pointer returned by virtualloc 
        Node *initial_head = static_cast<Node*>(memory_head_ptr);  

        list.init_head(initial_head, heap_size);

        list.print_head_ptr();

        cout << "Heap Initialized at: " << memory_head_ptr << endl;
            
    }


    void coalesce_recurse_forward(Node* ptr){
        size_t block_size = ptr->size;
        char* pointer = reinterpret_cast<char*>(ptr);
        char* next_mem_location = pointer + block_size;
        Node* _next_mem_location = reinterpret_cast<Node*>(next_mem_location);
        Node* prev{nullptr};
        Node* head = list.get_head_ptr();
        while(head){
            if (head == _next_mem_location){
                ptr->size = ptr->size + head->size;
                ptr->next = head->next;
                cout << "Recursion Worked. Onwards!" << endl;
                coalesce_recurse_forward(ptr);   // the "ptr" node should be bigger now
            }
            
            prev = head;
            head = head->next;
        }
}

    void coalesce_recurse_behind(Node* ptr){
        char* _ptr = reinterpret_cast<char*>(ptr);
        Node* head = list.get_head_ptr();
        Node* prev{nullptr};

        while(head){

            size_t size_to_next_mem_block = head->size;
            char* __ptr = _ptr + size_to_next_mem_block;
            Node* ___ptr = reinterpret_cast<Node*>(__ptr);

            if (___ptr == ptr){
                head->size = head->size + ptr->size;
                head->next = ptr->next;
                cout << "Block Successfully Coalesced_Behind_Recurse" << endl;
                coalesce_recurse_behind(___ptr);
            }

            prev = head;
            head = head->next;
        }
    }


    void return_freed_block(void* user_ptr){
        int block_successfully_returned = 0;

        char* temp_ptr = static_cast<char*>(user_ptr);
        char* find_base = temp_ptr - sizeof(Header);

        Header* base = reinterpret_cast<Header*>(find_base);
        size_t user_block_size = base->size;

        cout << "Size of Returned Block: " << user_block_size << endl;

        Header* copy_base = base;
        size_t total_size = copy_base->size + sizeof(Header);

        // coalsce if possible; find free memory in front or behind of returned block
        char* _base = reinterpret_cast<char*>(copy_base);
        cout << "base of header again "<< static_cast<void*>(_base) << endl;
        char* buddy_in_front_location = _base + total_size;
        //cout << "buddy_in_front_location" << static_cast<void*>(buddy_in_front_location) << endl;

        Node* returning_node = reinterpret_cast<Node*>(_base);
        returning_node->size = total_size;
        returning_node->next = nullptr;
        returning_node->memory_location = returning_node;
        Node* buddy_in_front = reinterpret_cast<Node*>(buddy_in_front_location);
        Node* head = list.get_head_ptr();
        Node* prev{nullptr};

        while(head){
            cout << "in the first while loop" << endl;
            if (head == buddy_in_front){
                returning_node->size = returning_node->size + head->size;
                returning_node->next = head->next;
                if (prev){
                    prev->next = returning_node;
                }

                block_successfully_returned = 1;
                cout << "Block successfully coalesced" << endl;

                coalesce_recurse_forward(returning_node);
                cout << "We are out of the recursive function!" << endl;
                break;
            }
            
            prev = head;
            head = head->next;
        }

        // check for buddy behind
        
        Node* new_head = list.get_head_ptr();

        while(new_head){
            cout << "got to second while loop" << endl;
            size_t size_to_next_mem_block = new_head->size ;
            cout << "size to next mem block " << size_to_next_mem_block << endl;
            char* _new_head = reinterpret_cast<char*>(new_head);
            char* ptr = _new_head + size_to_next_mem_block; // memory loaction right after the block : check to see if matches our returned block
            cout << "head ptr + size to next mem block " << static_cast<void*>(ptr) << endl;
            Node* __ptr = reinterpret_cast<Node*>(ptr);

            cout << "base of returned slab " << static_cast<void*>(_base) << endl;

            if (__ptr == (Node*)_base){

                new_head->size = new_head->size + total_size;

                block_successfully_returned = 1;

                cout << "Block Successful Coalesced_Behind" << endl;
                
                coalesce_recurse_behind(__ptr);
            }

            new_head = new_head->next;
        }

        if (block_successfully_returned == 0){
            Node* head = list.get_head_ptr();
            returning_node->next = head->next;
            head = returning_node;
            block_successfully_returned = 1;
            cout << "Block was added to front of list" << endl;
        }

        if (block_successfully_returned == 0){
            
            throw std::runtime_error("Could Not Return Block To Free List. Check return_freed_block function");
        }

    }


    
    void* add_header_information(void* user_slab, size_t size){
        Header* slab_base = static_cast<Header*>(user_slab);  // static_cast to add the size information
        
        slab_base->size = size - sizeof(Header);

        size_t header_offset = sizeof(Header);  

        char* base = reinterpret_cast<char*>(slab_base); // reinterpret for adding to the pointer

        char* temp_user_ptr = base + header_offset;
        
        void* user_ptr = static_cast<void*>(temp_user_ptr);

        //char* t = static_cast<char*>(user_ptr) - sizeof(Header);
        //cout << "Size of User Pointer: " << reinterpret_cast<Header*>(t)->size << endl;
        //cout << "User Pointer: " << user_ptr << endl;

        return user_ptr;
    }

    void* find_free_block(size_t size){
        size_t requested_bytes = size + sizeof(Header);
        Node* head = list.get_head_ptr();
        Node* prev_node{nullptr};

        while(head){
            if (head->size < requested_bytes){
                prev_node = head;
                head = head->next;

            } else if (head->size == requested_bytes){
                // this block is disappearing so prev->next must point to head->next
                if (head->next){
                    prev_node->next = head->next;
                }
                
                void* user_slab = static_cast<void*>(head);

                return add_header_information(user_slab, head->size);

            } else if (head->size > requested_bytes + sizeof(Node)){ // if we do not include Node size then splitting could lead lost info
                
                size_t temp_size = head->size;
                Node* temp_location = head->memory_location;
                Node* temp_next_location = head->next;

                char* base_location = reinterpret_cast<char*>(temp_location);

                size_t offset = temp_size - requested_bytes;

                char* user_slab_mem_location = base_location  + offset;
                
                void* user_slab = static_cast<void*>(user_slab_mem_location);

                // update size of remaining piece  
                head->size = head->size - requested_bytes;
                return add_header_information(user_slab, requested_bytes);
            }
        }

        throw std::bad_alloc();
    }


    size_t get_heapsize(){
        return heap_size;
    }

    private:
    Linked_List list{};
    void* memory_head_ptr{nullptr};
    size_t heap_size{4096};
};



// api

Heap heap{};

void user_heap_init(){
    Heap *hp = &heap;

    hp->heap_init();
}

void* user_allocate(size_t size){
    Heap *hp = &heap;

    if (size > hp->get_heapsize()){
        throw std::runtime_error("Size Requested Too Big");
        std::terminate();
    }

    void* free_block = hp->find_free_block(size);

    return free_block;
}

void user_free(void* ptr){
    Heap *hp = &heap;

    hp->return_freed_block(ptr);
}

void user_heap_destroy(){

}


