#include <Windows.h>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <map>

#define TOTALSIZE 0x100000000

extern "C" { __declspec(dllexport) uint64_t moduleBase; }


class XAlloc
{
public:
    XAlloc() {
        base = VirtualAlloc(nullptr, TOTALSIZE, MEM_RESERVE, PAGE_NOACCESS);
        if (!base) {
            throw std::runtime_error("Failed to reserve memory");
        }
        moduleBase = reinterpret_cast<uint64_t>(base);
        // Initially, the entire reserved region is free.
        freeBlocks.push_back({ 0, TOTALSIZE });
    }

   
    void* allocate(size_t size, size_t preferredOffset = SIZE_MAX) 
    {
        size = alignSize(size);
        size_t allocOffset = 0;
        bool found = false;

        
        if (preferredOffset != SIZE_MAX) 
        {
            for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) 
            {
                if (it->offset <= preferredOffset && (it->offset + it->size) >= (preferredOffset + size)) 
                {
                    allocOffset = preferredOffset;
                    splitFreeBlock(it, allocOffset, size);
                    found = true;
                    break;
                }
            }
        }
        
        if (!found) {
            for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) 
            {
                if (it->size >= size) 
                {
                    allocOffset = it->offset;
                    splitFreeBlock(it, allocOffset, size);
                    found = true;
                    break;
                }
            }
        }
        if (!found) 
        {
            return nullptr;  
        }
        
        void* addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(base) + allocOffset);
        void* result = VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE);
        if (!result) 
        {
            throw std::runtime_error("VirtualAlloc commit failed");
        }
        return result;
    }

   
    void free(void* ptr, size_t size) 
    {
        size = alignSize(size);
        size_t offset = reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(base);
        // Decommit the pages.
        if (!VirtualFree(ptr, size, MEM_DECOMMIT)) 
        {
            throw std::runtime_error("VirtualFree decommit failed");
        }
        
        freeBlocks.push_back({ offset, size });
        mergeFreeBlocks();
    }

    
    void* resize(void* ptr, size_t oldSize, size_t newSize) 
    {
        newSize = alignSize(newSize);
        oldSize = alignSize(oldSize);

        if (newSize <= oldSize) 
        {
            
            size_t excess = oldSize - newSize;
            if (excess > 0) 
            {
                void* excessAddr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) + newSize);
                VirtualFree(excessAddr, excess, MEM_DECOMMIT);
                size_t offset = reinterpret_cast<uintptr_t>(excessAddr) - reinterpret_cast<uintptr_t>(base);
                freeBlocks.push_back({ offset, excess });
                mergeFreeBlocks();
            }
            return ptr;
        }
        else 
        {
            
            size_t offset = reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(base);
            size_t desiredOffset = offset + oldSize;
            for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) 
            {
                if (it->offset == desiredOffset && it->size >= (newSize - oldSize)) 
                {
                    void* addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(base) + desiredOffset);
                    void* res = VirtualAlloc(addr, newSize - oldSize, MEM_COMMIT, PAGE_READWRITE);
                    if (res) 
                    {
                        splitFreeBlock(it, desiredOffset, newSize - oldSize);
                        return ptr;
                    }
                    break;  
                }
            }
            
            void* newPtr = allocate(newSize);
            if (newPtr) 
            {
                memcpy(newPtr, ptr, oldSize);
                free(ptr, oldSize);
            }
            return newPtr;
        }
    }

    void write64To(uint32_t guest, uint64_t val) { *(uint64_t*)((uint64_t)base + guest) = val; }
    void write32To(uint32_t guest, uint32_t val) { *(uint64_t*)((uint64_t)base + guest) = val; }
    void write16To(uint32_t guest, uint16_t val) { *(uint64_t*)((uint64_t)base + guest) = val; }
    void write8To(uint32_t guest, uint8_t val) { *(uint64_t*)((uint64_t)base + guest) = val; }

    uint64_t read64From(uint32_t guest) { return (uint64_t)(*(uint64_t*)((uint64_t)base + guest)); }
    uint32_t read32From(uint32_t guest) { return (uint32_t)(*(uint64_t*)((uint64_t)base + guest)); }
    uint16_t read16From(uint32_t guest) { return (uint16_t)(*(uint64_t*)((uint64_t)base + guest)); }
    uint8_t read8From(uint32_t guest) { return (uint8_t)(*(uint64_t*)((uint64_t)base + guest)); }

    uint32_t makeHostGuest(void* host) { return (uint64_t)host - (uint64_t)base;  }

    ~XAlloc() 
    {
        if (base) 
        {
            VirtualFree(base, 0, MEM_RELEASE);
        }
    }

private:
    
    struct FreeBlock 
    {
        size_t offset;
        size_t size;
    };

    std::vector<FreeBlock> freeBlocks;
    void* base = nullptr;
    const size_t pageSize = 4096;

    
    size_t alignSize(size_t size) 
    {
        return (size + pageSize - 1) & ~(pageSize - 1);
    }

    
    void splitFreeBlock(std::vector<FreeBlock>::iterator it, size_t allocOffset, size_t allocSize) 
    {
        FreeBlock block = *it;
        freeBlocks.erase(it);
      
        if (allocOffset > block.offset) 
        {
            freeBlocks.push_back({ block.offset, allocOffset - block.offset });
        }
  
        size_t endAlloc = allocOffset + allocSize;
        size_t blockEnd = block.offset + block.size;
        if (endAlloc < blockEnd) 
        {
            freeBlocks.push_back({ endAlloc, blockEnd - endAlloc });
        }
        mergeFreeBlocks();
    }

  
    void mergeFreeBlocks() 
    {
        std::sort(freeBlocks.begin(), freeBlocks.end(), [](const FreeBlock& a, const FreeBlock& b) 
            {
            return a.offset < b.offset;
            });
        std::vector<FreeBlock> merged;
        for (const auto& block : freeBlocks) 
        {
            if (!merged.empty() && (merged.back().offset + merged.back().size == block.offset)) 
            {
                merged.back().size += block.size;
            }
            else 
            {
                merged.push_back(block);
            }
        }
        freeBlocks = std::move(merged);
    }
};