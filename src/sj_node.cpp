#include "sj_node.hpp"

#include <cmath>
#include <algorithm>

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
    default:
        break;
    }
}

Node::Node(IObjectValue *p)
{
    if (p != nullptr)
    {
        p->retain();
        type_ = p->getType();
    }
    else
    {
        type_ = T_NULL;
    }
    value_.p = p;
}

const Node& Node::operator = (const Node &other)
{
    if (other.isPointer())
    {
        return *this = other.value_.p;
    }
    else
    {
        safeRelease();
        type_ = other.type_;
        value_ = other.value_;
        return *this;
    }
}

const Node& Node::operator = (Node &&other)
{
    safeRelease();
    type_ = other.type_;
    value_ = other.value_;
    other.type_ = T_NULL;
    return *this;
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
    return rawArray();
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
    return rawDict();
}

size_t Node::size() const
{
    if (isArray())
    {
        return refArray().size();
    }
    else if (isDict())
    {
        return refDict().size();
    }
    else if (isString())
    {
        return value_.ps->size();
    }
    return 0;
}

size_t Node::capacity() const
{
    if (isArray())
    {
        return refArray().capacity();
    }
    return 0;
}

void Node::reserve(size_t capacity)
{
    if (isArray())
    {
        refArray().reserve(capacity);
    }
    else if (isDict())
    {
        refDict().reserve(capacity);
    }
}

void Node::clear()
{
    if (isArray())
    {
        refArray().clear();
    }
    else if (isDict())
    {
        refDict().clear();
    }
}

Node Node::clone() const
{
    Node ret;

    ret.type_ = type_;
    ret.value_ = value_;
    if (isPointer())
    {
        ret.value_.p = value_.p->clone();
        ret.value_.p->retain();
    }
    return ret;
}

Node Node::deepClone() const
{
    Node ret;

    ret.type_ = type_;
    ret.value_ = value_;
    if (isPointer())
    {
        ret.value_.p = value_.p->deepClone();
        ret.value_.p->retain();
    }
    return  ret;
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

bool Node::operator < (const Node &other) const
{
    if (this->isNumber() && other.isNumber())
    {
        if (this->isFloat() && other.isFloat())
        {
            return value_.f < other.value_.f;
        }
        else if (this->isFloat())
        {
            return value_.f < (Float)other.value_.i;
        }
        else if (other.isFloat())
        {
            return (Float)value_.i < other.value_.f;
        }
        else
        {
            return value_.i < other.value_.i;
        }
    }

    if (type_ != other.type_)
    {
        return type_ < other.type_;
    }

    if (type_ == T_NULL)
    {
        return true;
    }
    else if (type_ == T_BOOL)
    {
        return value_.b < other.value_.b;
    }
    else if (type_ == T_STRING)
    {
        return rawString()->compare(other.rawString()) < 0;
    }
    else if (type_ == T_ARRAY)
    {
        return rawArray() < other.rawArray();
    }
    else if (type_ == T_DICT)
    {
        return rawDict() < other.rawDict();
    }
    else
    {
        SJ_ASSERT(false && "shouldn't reach here.");
        return false;
    }
}

static std::hash<bool> s_boolHash;
static std::hash<Integer> s_intHash;
static std::hash<Float> s_floatHash;

size_t Node::getHash() const
{
    switch (type_)
    {
    case T_NULL:
        return 0;
    case T_BOOL:
        return s_boolHash(value_.b);
    case T_INT:
        return s_intHash(value_.i);
    case T_FLOAT:
        return s_floatHash(value_.f);
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
    
    static Node _null;
    _null.setNull();
    return _null;
}

const Node& Node::findMemberByPath(const char *keyPath, size_t keyLength) const
{
    const Node *pNode = this;

    const char *begin = keyPath;
    const char *end = begin + keyLength;

    // 注意：如果begin = end，说明是空字符串。空字符串也是可以作为key的
    while (begin <= end)
    {
        if (!pNode->isDict())
        {
            return nullValue();
        }

        // 注意：如果没有找到'/'，pos依然是合法的指针，此时pos = end
        const char *pos = std::find(begin, end, '/');

        DictValue *dict = pNode->value_.pd;
        StringValue *name = dict->getAllocator()->createString(begin, pos - begin, BT_NOT_CARE);
        auto iterator = dict->imp.find(Node(name));
        if (iterator == dict->imp.end())
        {
            return nullValue();
        }

        pNode = &iterator->second;
        begin = pos + 1; // skip '/'
    }

    return *pNode;
}

bool Node::hasMemberByPath(const char *keyPath, size_t keyLength) const
{
    const Node *pNode = this;

    const char *begin = keyPath;
    const char *end = begin + keyLength;

    // 注意：如果begin = end，说明是空字符串。空字符串也是可以作为key的
    while (begin <= end)
    {
        if (!pNode->isDict())
        {
            return false;
        }

        // 注意：如果没有找到'/'，pos依然是合法的指针，此时pos = end
        const char *pos = std::find(begin, end, '/');

        DictValue *dict = pNode->value_.pd;
        StringValue *name = dict->getAllocator()->createString(begin, pos - begin, BT_NOT_CARE);
        auto iterator = dict->imp.find(Node(name));
        if (iterator == dict->imp.end())
        {
            return false;
        }

        pNode = &iterator->second;
        begin = pos + 1; // skip '/'
    }

    return true;
}

bool Node::setMemberByPath(const char *keyPath, size_t keyLength, const Node &val)
{
    Node *pNode = this;

    const char *begin = keyPath;
    const char *end = begin + keyLength;
    while (begin <= end)
    {
        if (!pNode->isDict())
        {
            return false;
        }

        // 注意：如果没有找到'/'，pos依然是合法的指针，此时pos = end
        const char *pos = std::find(begin, end, '/');

        DictValue *dict = pNode->value_.pd;
        Node name = dict->getAllocator()->createString(begin, pos - begin, BT_MAKE_COPY);
        auto iterator = dict->imp.find(name);
        if (iterator != dict->imp.end())
        {
            pNode = &iterator->second;
        }
        else
        {
            pNode = &dict->imp[name];
            if (pos != end)
            {
                pNode->setDict(dict->getAllocator());
            }
        }
        begin = pos + 1; // skip '/'
    }

    *pNode = val;
    return true;
}

bool Node::removeMemberByPath(const char * keyPath, size_t keyLength)
{
    Node *pNode = this;

    const char *begin = keyPath;
    const char *end = begin + keyLength;
    while (begin <= end)
    {
        if (!pNode->isDict())
        {
            return false;
        }

        // 注意：如果没有找到'/'，pos依然是合法的指针，此时pos = end
        const char *pos = std::find(begin, end, '/');

        DictValue *dict = pNode->value_.pd;
        StringValue *name = dict->getAllocator()->createString(begin, pos - begin, BT_NOT_CARE);
        auto iterator = dict->imp.find(Node(name));
        if (iterator == dict->imp.end())
        {
            return false;
        }

        if (pos == end)
        {
            dict->imp.erase(iterator);
            return true;
        }

        pNode = &iterator->second;
        begin = pos + 1; // skip '/'
    }

    return false;
}

bool Node::hasMember(const Node &key) const
{
    if (key.isString())
    {
        StringValue *s = key.rawString();
        return hasMemberByPath(s->data(), s->size());
    }
    else if (isDict())
    {
        return findMember(key) != refDict().end();
    }
    else
    {
        return false;
    }
}

void Node::setMember(const Node &key, const Node &val)
{
    if (key.isString())
    {
        StringValue *s = key.rawString();
        setMemberByPath(s->data(), s->size(), val);
    }
    else if (isDict())
    {
        refDict()[key] = val;
    }
}

bool Node::removeMember(const Node &key)
{
    if (key.isString())
    {
        StringValue *s = key.rawString();
        return removeMemberByPath(s->data(), s->size());
    }
    else if (isDict())
    {
        return value_.pd->remove(key);
    }
    else
    {
        return false;
    }
}

const Node& Node::operator[] (const char *key) const
{
    if (isDict())
    {
        return findMemberByPath(key, strlen(key));
    }
    else if (isArray() && 0 == key) // node[0]会进入了当前函数
    {
        if (!refArray().empty())
        {
            return refArray()[0];
        }
    }
    return nullValue();
}

const Node& Node::operator[] (const std::string &key) const
{
    if (isDict())
    {
        return findMemberByPath(key.c_str(), key.size());
    }
    return nullValue();
}

const Node& Node::operator[] (const Node &key) const
{
    if (isDict())
    {
        if (key.isString())
        {
            StringValue *s = key.rawString();
            return findMemberByPath(s->data(), s->size());
        }

        ConstDictIterator it = findMember(key);
        if (it != memberEnd())
        {
            return it->second;
        }
    }
    return nullValue();
}

void Node::getKeys(std::vector<Node>& output, bool sort) const
{
    const Dict &dict = refDict();
    output.clear();
    output.reserve(dict.size());
    for (auto &pair : dict)
    {
        output.push_back(pair.first);
    }

    if (sort)
    {
        std::sort(output.begin(), output.end());
    }
}

void Node::getValues(std::vector<Node>& output) const
{
    const Dict &dict = refDict();
    output.clear();
    output.reserve(dict.size());
    for (auto &pair : dict)
    {
        output.push_back(pair.second);
    }
}

void Node::getMembers(std::vector<NodePair>& output, bool sort) const
{
    const Dict &dict = refDict();
    output.clear();
    output.reserve(dict.size());
    for (auto &pair : dict)
    {
        output.push_back(pair);
    }

    if (sort)
    {
        std::sort(output.begin(), output.end(), compareMember);
    }
}

NS_SMARTJSON_END
