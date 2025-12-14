#pragma once

#include <cstddef>
#include <vector>

using namespace std;

class Pool {
    public:
        Pool(size_t chunkSize, size_t capacity)
            : chunkSize(chunkSize), capacity(capacity),
              buffer(chunkSize * capacity), freeList(nullptr), used(0), freeListSize(0) {}
    
        void* alloc() {
            if (freeList) {
                void* p = freeList;
                freeList = *reinterpret_cast<void**>(freeList);
                freeListSize--;
                return p;
            }
            if (used >= capacity) return nullptr; // out of memory
            void* p = buffer.data() + used * chunkSize;
            ++used;
            return p;
        }
    
        void free(void* p) {
            void** nextPtr = reinterpret_cast<void**>(p);
            *nextPtr = freeList;
            freeList = p;
            freeListSize++;
            // cout << "allocated: " << (used - freeListSize) << endl;
        }
    
    private:
        size_t chunkSize;
        size_t capacity;
        std::vector<std::max_align_t> buffer;
    
        void* freeList;  // singly linked list of free blocks, stored inside blocks
        size_t used;
        size_t freeListSize;
    };
    