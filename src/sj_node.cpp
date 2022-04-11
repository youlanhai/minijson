#include "sj_node.hpp"

#include <cmath>

NS_SMARTJSON_BEGIN

//////////////////////////////////////////////////////////////////
// IAllocator
//////////////////////////////////////////////////////////////////
static char s_emptyStrBuffer[1] = { 0 };

StringValue* IAllocator::createString(const char *str, size_t size, BufferType type)
{
    if (nullptr == str || 0 == size)
    {
        str = s_emptyStrBuffer;
        size = 0;
        type = BT_NOT_CARE;
    }

    if (type == BT_MAKE_COPY)
    {
        char *buffer = new char[sizeof(StringValue) + size + 1];
        char *s = buffer + sizeof(StringValue);
        memcpy(s, str, size);
        s[size] = 0;
        // placement new
        return new(buffer)StringValue(s, size, this);
    }
    else
    {
        return new StringValue(str, size, this);
    }
}

ArrayValue* IAllocator::createArray(size_t capacity)
{
    ArrayValue *ret = new ArrayValue(this);
    if (capacity > 0)
    {
        ret->imp.reserve(capacity);
    }
    return ret;
}

DictValue* IAllocator::createDict(size_t capacity)
{
    DictValue *ret = new DictValue(this);
    if (capacity > 0)
    {
        ret->imp.reserve(capacity);
    }
    return ret;
}

void IAllocator::freeObject(IObjectValue *p)
{
    this->retain();
    
    delete p;
    
    this->release();
}

static IAllocator* createDefaultAllocator()
{
    static IAllocator s_allocator;
    s_allocator.retain();
    s_allocator.retain(); // 避免被意外释放
    return &s_allocator;
}

/*static*/ IAllocator* IAllocator::s_pDefault = createDefaultAllocator();

/*static*/ void IAllocator::setDefaultAllocator(IAllocator *p)
{
    if(p != nullptr)
    {
        p->retain();
    }
    
    if(s_pDefault)
    {
        s_pDefault->release();
    }
    
    s_pDefault = p;
}

//////////////////////////////////////////////////////////////////
// IObjectValue
//////////////////////////////////////////////////////////////////

IObjectValue::IObjectValue(IAllocator *allocator)
: allocator_(allocator)
{
    allocator_->retain();
}

IObjectValue::~IObjectValue()
{
    allocator_->release();
}

void IObjectValue::destroyThis()
{
    allocator_->freeObject(this);
}

//////////////////////////////////////////////////////////////////
// StringValue
//////////////////////////////////////////////////////////////////

#if !SJ_PLATFORM_64BIT
static uint32_t MurmurHash2 ( const void * key, int len, uint32_t seed )
{
    /* 'm' and 'r' are mixing constants generated offline.
         They're not really 'magic', they just happen to work well.  */

    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a 'random' value */

    uint32_t h = seed ^ len;

    /* Mix 4 bytes at a time into the hash */

    const unsigned char * data = (const unsigned char *)key;

    while(len >= 4)
    {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */

    switch(len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
    };

    /* Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.  */

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
} 
#else

#define BIG_CONSTANT(x) (x##LLU)

/*-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby
//
// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
// and endian-ness issues if used across multiple platforms.
//
// 64-bit hash for 64-bit platforms
*/

static uint64_t MurmurHash2 ( const void * key, int len, uint64_t seed )
{
    const uint64_t m = BIG_CONSTANT(0xc6a4a7935bd1e995);
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t * data = (const uint64_t *)key;
    const uint64_t * end = data + (len/8);

    while(data != end)
    {
        uint64_t k = *data++;

        k *= m; 
        k ^= k >> r; 
        k *= m; 
        
        h ^= k;
        h *= m; 
    }

    const unsigned char * data2 = (const unsigned char*)data;

    switch(len & 7)
    {
    case 7: h ^= ((uint64_t) data2[6]) << 48;
    case 6: h ^= ((uint64_t) data2[5]) << 40;
    case 5: h ^= ((uint64_t) data2[4]) << 32;
    case 4: h ^= ((uint64_t) data2[3]) << 24;
    case 3: h ^= ((uint64_t) data2[2]) << 16;
    case 2: h ^= ((uint64_t) data2[1]) << 8;
    case 1: h ^= ((uint64_t) data2[0]);
                    h *= m;
    };
 
    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}
#endif

static size_t s_seed;

StringValue::StringValue(const char *str, size_t size, IAllocator *allocator)
    : IObjectValue(allocator)
    , str_(str)
    , size_(size)
    , hash_((size_t)-1)
    , bufferType_(BT_NOT_CARE)
{
}

size_t StringValue::computeHash() const
{
    return (size_t)MurmurHash2(str_, (int)size_, (size_t)&s_seed);
}

int StringValue::compare(const char *str, size_t length) const
{
    size_t minSize = size_ < length ? size_ : length;
    int ret = memcmp(str_, str, minSize);
    if (ret == 0)
    {
        return (int)size_ - (int)length;
    }
    return ret;
}

//////////////////////////////////////////////////////////////////
// ArrayValue
//////////////////////////////////////////////////////////////////

IObjectValue* ArrayValue::deepClone() const
{
    ArrayValue *ret = allocator_->createArray(imp.size());
    for (const Node &v : imp)
    {
        ret->imp.push_back(v.deepClone());
    }
    return ret;
}

Array::iterator ArrayValue::find(const Node &value)
{
    for (auto it = imp.begin(); it != imp.end(); ++it)
    {
        if (value == *it)
        {
            return it;
        }
    }
    return imp.end();
}

//////////////////////////////////////////////////////////////////
// DictValue
//////////////////////////////////////////////////////////////////

IObjectValue* DictValue::deepClone() const
{
    DictValue *ret = allocator_->createDict(imp.size());
    for (const auto &pair : imp)
    {
        Node key = pair.first.deepClone();
        Node val = pair.second.deepClone();
        ret->imp[key] = val;
    }
    return ret;
}

//////////////////////////////////////////////////////////////////
// Node
//////////////////////////////////////////////////////////////////
Node Node::s_null;

Node::Node(ValueType type, IAllocator *allocator)
    : type_(T_NULL)
{
    switch (type)
    {
    case T_BOOL:
        value_.b = false;
        type_ = T_BOOL;
        break;
    case T_INT:
        value_.i = 0;
        type_ = T_INT;
        break;
    case T_FLOAT:
        value_.f = 0;
        type_ = T_FLOAT;
        break;
    case T_STRING:
        setString("", 0, allocator);
        break;
    case T_ARRAY:
        setArray(allocator);
        break;
    case T_DICT:
        setDict(allocator);
        break;
    }
}

void Node::setObject(const IObjectValue *value)
{
    if (!isPointer() || value_.p != value)
    {
        setNull();
        if (value != nullptr)
        {
            type_ = value->getType();
            value_.p = const_cast<IObjectValue*>(value);
            value_.p->retain();
        }
    }
}

void Node::setString(const char *str, size_t size, IAllocator *allocator)
{
    if (nullptr == str)
    {
        size = 0;
    }
    else if(0 == size)
    {
        size = strlen(str);
    }
    
    if(nullptr == allocator)
    {
        allocator = isPointer() ? (IAllocator*)value_.p->getAllocator() : IAllocator::getDefaultAllocator();
    }
    allocator->retain();
    
    safeRelease();
    type_ = T_STRING;
    value_.p = allocator->createString(str, size, BT_MAKE_COPY);
    value_.p->retain();
    
    allocator->release();
}

Array* Node::setArray(IAllocator *allocator)
{
    if(nullptr == allocator)
    {
        allocator = isPointer() ? (IAllocator*)value_.p->getAllocator() : IAllocator::getDefaultAllocator();
    }
    allocator->retain();
    
    safeRelease();
    type_ = T_ARRAY;
    value_.p = allocator->createArray(0);
    value_.p->retain();
    
    allocator->release();
    return (Array*)value_.p;
}

Dict* Node::setDict(IAllocator *allocator)
{
    if(nullptr == allocator)
    {
        allocator = isPointer() ? (IAllocator*)value_.p->getAllocator() : IAllocator::getDefaultAllocator();
    }
    allocator->retain();

    safeRelease();
    type_ = T_DICT;
    value_.p = allocator->createDict(0);
    value_.p->retain();
    
    allocator->release();
    return (Dict*)value_.p;
}

bool Node::operator == (const Node &other) const
{
    if(this->isNumber() && other.isNumber())
    {
        if(this->isFloat() && other.isFloat())
        {
            return fabs(value_.f - other.value_.f) < DefaultEpsilon;
        }
        else if(this->isFloat())
        {
            return fabs(value_.f - (Float)other.value_.i) < DefaultEpsilon;
        }
        else if(other.isFloat())
        {
            return fabs((Float)value_.i - other.value_.f) < DefaultEpsilon;
        }
        else
        {
            return value_.i == other.value_.i;
        }
    }
    
    if(type_ != other.type_)
    {
        return false;
    }
    
    if(type_ == T_NULL)
    {
        return true;
    }
    else if(type_ == T_BOOL)
    {
        return value_.b == other.value_.b;
    }
    else if(type_ == T_STRING)
    {
        return rawString()->compare(other.rawString()) == 0;
    }
    else if(type_ == T_ARRAY)
    {
        return refArray() == other.refArray();
    }
    else if(type_ == T_DICT)
    {
        return refDict() == other.refDict();
    }
    else
    {
        SJ_ASSERT(false && "shouldn't reach here.");
        return false;
    }
}

size_t Node::getHash() const
{
    switch (type_)
    {
    case T_NULL:
        return 0;
    case T_BOOL:
        return std::hash<bool>()(value_.b);
    case T_INT:
        return std::hash<Integer>()(value_.i);
    case T_FLOAT:
        return std::hash<Float>()(value_.f);
    case T_STRING:
        return value_.ps->getHash();
    case T_ARRAY:
        return (size_t)value_.p;
    case T_DICT:
        return (size_t)value_.p;
    default:
        return 0;
    }
}

Node& Node::operator[] (size_t index)
{
    if(isArray())
    {
        if(index < rawArray()->size())
        {
            return refArray()[index];
        }
    }
    else if(isDict())
    {
        Dict::iterator it = rawDict()->find(Node(index));
        if(it != rawDict()->end())
        {
            return it->second;
        }
    }
    
    return nullValue();
}

Integer Node::asInteger() const
{
    if (isInt()) return value_.i;
    if (isFloat()) return static_cast<Integer>(value_.f);
    return 0;
}

UInteger Node::asUInteger() const
{
    if (isInt()) return value_.u;
    if (isFloat()) return static_cast<UInteger>(static_cast<Integer>(value_.f));
    return 0;
}

NS_SMARTJSON_END
