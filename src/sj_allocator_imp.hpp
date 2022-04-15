#pragma once
#include "sj_node.hpp"

NS_SMARTJSON_BEGIN

/**
 *  Allocate small memorty by using memory pool.
 *  May be efficent, but not thread safe.
 *  Still in testing stage.
 */
class MemoryPoolAllocator : public IAllocator
{
public:
    explicit MemoryPoolAllocator(size_t pageSize = 100 * 1024);
    ~MemoryPoolAllocator();
    
    void*   malloc(size_t size);
    void    free(void *p);

    StringValue* createString(const char *str, size_t size, BufferType type) override;
    ArrayValue* createArray(size_t capacity) override;
    DictValue* createDict(size_t capacity) override;
    
    void freeObject(IObjectValue *p) override;
    
private:
    struct MemNode
    {
        MemNode* next;
    };
    
    struct MemSlot
    {
        char*       addr;
        MemNode*    freeList;
        MemSlot*    next;
        int         allocCount;
    };
    
    MemSlot* newSlot();
    void tryFreeSlot(MemSlot *s);
    
private:
    MemSlot*    slot_;

    size_t      pageSize_;
    
    // This allocator only allocate memory whitch size <= minAllocSize_.
    size_t      minAllocSize_;
};

NS_SMARTJSON_END
