#pragma once

#include <cstddef>
#include <vector>
#include <cstdint>

using namespace std;

class Pool {
    public:
        Pool(size_t chunkSize, size_t capacity)
            : chunkSize(chunkSize), capacity(capacity),
              buffer((chunkSize * capacity + sizeof(std::max_align_t) - 1) / sizeof(std::max_align_t)), freeList(nullptr), used(0) {}
    
        void* alloc() {
            if (freeList) {
                void* p = freeList;
                freeList = *reinterpret_cast<void**>(freeList);
                return p;
            }
            if (used >= capacity) return nullptr; // out of memory
            void* p = reinterpret_cast<char*>(buffer.data()) + used * chunkSize;
            ++used;
            return p;
        }
    
        void free(void* p) {
            if (p == nullptr) return;  // Safety check
            void** nextPtr = reinterpret_cast<void**>(p);
            *nextPtr = freeList;
            freeList = p;
        }
    
    private:
        size_t chunkSize;
        size_t capacity;
        std::vector<std::max_align_t> buffer;  // Use max_align_t for proper alignment
    
        void* freeList;  // singly linked list of free blocks, stored inside blocks
        size_t used;
    };
    