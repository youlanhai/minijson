#pragma once
#include "sj_config.hpp"

#include <cfloat>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>

NS_SMARTJSON_BEGIN

#if SJ_USE_LARGE_NUMBER
typedef int64_t     Integer;
typedef uint64_t    UInteger;
typedef double      Float;
const Float DefaultEpsilon = DBL_EPSILON;
#else
typedef int32_t     Integer;
typedef int32_t     UInteger;
typedef float       Float;
const Float DefaultEpsilon = FLT_EPSILON;
#endif

enum ValueType
{
    T_NULL,
    T_BOOL,
    T_INT,
    T_FLOAT,
    
    T_POINTER,
    T_STRING,
    T_ARRAY,
    T_DICT,
};

enum BufferType
{
    BT_NOT_CARE, // IObjectValue use the buffer freely, caller free buffer
    BT_MAKE_COPY, // IObjectValue should make a copy of the buffer
};

enum ResultCode
{
    RC_OK,
    RC_END_OF_FILE,
    RC_OPEN_FILE_ERROR,
    RC_FILE_EMPTY,
    /** 不是有效的json文件格式 */
    RC_INVALID_JSON,
    /** 无效的字典格式 */
    RC_INVALID_DICT,
    /** 无效的字典key */
    RC_INVALID_KEY,
    RC_INVALID_ARRAY,
    RC_INVALID_STRING,
    RC_INVALID_NUMBER,
    RC_INVALID_NULL,
    RC_INVALID_TRUE,
    RC_INVALID_FALSE,
    RC_INVALID_TYPE,
    /** 注释格式错误 */
    RC_INVALID_COMMENT,
    /** 无效的字符格式。\x格式需要2个16进制字符，如: \xab */
    RC_INVALID_CHAR,
    /** 无效的Unicode字符格式。\u格式需要4个16进制字符，如: \uabcd */
    RC_INVALID_UNICODE,
};

// predefine

class Node;
class IAllocator;

class IObjectValue;
class StringValue;
class ArrayValue;
class DictValue;

NS_SMARTJSON_END

namespace std
{
    template<>
    struct hash<NS_SMARTJSON Node>
    {
        typedef NS_SMARTJSON Node argument_type;
        typedef size_t result_type;
        size_t operator()(const NS_SMARTJSON Node &node) const noexcept;
    };
}

NS_SMARTJSON_BEGIN

typedef std::unordered_map<Node, Node> Dict;
typedef std::vector<Node> Array;

typedef Array::iterator ArrayIterator;
typedef Array::const_iterator ConstArrayIterator;

typedef Dict::iterator DictIterator;
typedef Dict::const_iterator ConstDictIterator;

typedef std::pair<Node, Node> NodePair;

/** 引用计数基类 */
class IRefCout
{
    SJ_DISABLE_COPY_ASSIGN(IRefCout);
public:
    IRefCout() : refCount_(0) {}
    virtual ~IRefCout() {}

    long getRefCount() const { return refCount_; }
    void retain() { ++refCount_; }

    void release()
    {
        SJ_ASSERT(refCount_ > 0);
        if (--refCount_ <= 0)
        {
            destroyThis();
        }
    }

    virtual void destroyThis() { delete this; }

private:
    long refCount_;
};

class IAllocator : public IRefCout
{
public:
    IAllocator() = default;
    ~IAllocator() = default;
    
    virtual StringValue* createString(const char *str, size_t size, BufferType type);
    virtual ArrayValue* createArray(size_t capacity);
    virtual DictValue* createDict(size_t capacity);
    
    virtual void freeObject(IObjectValue *p);
    
    static void setDefaultAllocator(IAllocator *p);
    static IAllocator* getDefaultAllocator(){ return s_pDefault; }
    
private:
    static IAllocator *s_pDefault;
};

class IObjectValue : public IRefCout
{
    SJ_DISABLE_COPY_ASSIGN(IObjectValue);
public:
    explicit IObjectValue(IAllocator *allocator);
    ~IObjectValue();
    
    virtual void destroyThis() override;
    virtual ValueType getType() const = 0;
    virtual IObjectValue* clone() const = 0;
    virtual IObjectValue* deepClone() const = 0;
    
    inline IAllocator* getAllocator(){ return allocator_; }
    
protected:
    IAllocator* allocator_;
};

class StringValue : public IObjectValue
{
    SJ_DISABLE_COPY_ASSIGN(StringValue);
public:
    StringValue(const char *str, size_t size, IAllocator *allocator);
    ~StringValue() = default;
    
    size_t size() const { return size_; }

    const char* data() const { return str_; }

    ValueType getType() const { return T_STRING; }

    int compare(const char *str, size_t length) const;
    int compare(const char *str) const { return compare(str, strlen(str)); }
    int compare(const StringValue *p) const { return compare(p->str_, p->size_); }
    int compare(const std::string &str) const { return compare(str.c_str(), str.size()); }
    
    //string is constant, deosn't need clone.
    IObjectValue* clone() const { return const_cast<StringValue*>(this); }
    IObjectValue* deepClone() const {return clone(); }

    size_t getHash() const
    {
        if (hash_ == (size_t)-1)
        {
            hash_ = computeHash();
        }
        return hash_;
    }

    size_t computeHash() const;
    void to(std::string &output) { output.assign(str_, size_); }
    
private:
    const char*     str_;
    size_t          size_;
    mutable size_t  hash_;
};

class Node
{
public:
    Node() : type_(T_NULL) {}
    ~Node() { safeRelease(); }

    Node(const Node &other);
    Node(Node &&other);

    template <typename T>
    Node(const T &value);

    Node(IObjectValue *p);

    Node(const char *str, size_t size, IAllocator *allocator = nullptr);
    Node(const std::string &value, IAllocator *allocator = 0);

    explicit Node(ValueType type, IAllocator *allocator = 0);

    ValueType getType() const { return type_; }

    bool isNull()   const { return type_ == T_NULL; }
    bool isBool()   const { return type_ == T_BOOL; }
    bool isInt()    const { return type_ == T_INT; }
    bool isFloat()  const { return type_ == T_FLOAT; }
    bool isString() const { return type_ == T_STRING; }
    bool isArray()  const { return type_ == T_ARRAY; }
    bool isDict()   const { return type_ == T_DICT; }
    bool isNumber() const { return type_ == T_INT || type_ == T_FLOAT; }
    bool isPointer() const { return type_ > T_POINTER; }

    bool        asBool()    const;
    Integer     asInteger() const;
    UInteger    asUInteger()const;
    Float       asFloat()   const;
    StringValue*asString()  const;
    const char* asCString() const;
    Array*      asArray()   const;
    Dict*       asDict()    const;

    template <typename T>
    T as() const;

    //{ NOTICE the `raw*` and `ref*` method was not safe.
    bool        rawBool() const;
    Integer     rawInteger() const;
    UInteger    rawUInteger() const;
    Float       rawFloat() const;
    const char* rawCString() const;
    StringValue*rawString() const;
    Array*      rawArray() const;
    Dict*       rawDict() const;

    StringValue* rawString();
    Array*      rawArray();
    Dict*       rawDict();

    const StringValue& refString() const;
    const Array& refArray() const;
    const Dict&  refDict() const;

    StringValue& refString();
    Array&      refArray();
    Dict&       refDict();

    //}


    void        setNull();
    void        setBool(bool v);
    void        setInteger(Integer v);
    void        setUInteger(UInteger v);
    void        setFloat(Float v);
    void        setObject(const IObjectValue *p);
    void        setString(const char *str, size_t size = 0, IAllocator *allocator = 0);
    Array*      setArray(IAllocator *allocator = 0);
    Dict*       setDict(IAllocator *allocator = 0);

    template <typename T>
    const Node& operator = (const T& value);
    const Node& operator = (const Node &value);
    const Node& operator = (Node &&value);

    bool operator == (const Node &value) const;
    bool operator != (const Node &value) const { return !(*this == value); }
    bool operator < (const Node &value) const;

    Node clone() const;
    Node deepClone() const;
    size_t getHash() const;

    IAllocator* getAllocator() const;

public: // array and dict

    size_t size() const;
    size_t capacity() const;
    void reserve(size_t capacity);
    void clear();

public: // array

    ArrayIterator begin();
    ConstArrayIterator begin() const;

    ArrayIterator end();
    ConstArrayIterator end() const;

    void resize(size_t size);

    Node& front();
    const Node& front() const;

    Node& back();
    const Node& back() const;

    void pushBack(const Node &node);
    void popBack();

    ArrayIterator find(const Node &node);
    ConstArrayIterator find(const Node &node) const;

    void insert(ArrayIterator it, const Node &node);
    void erase(ArrayIterator it);
    void remove(const Node &node);

    /** index array, or get member from dict.
     *  @param index  index of array, or key of dict.
     *  @return return value if the index is in range of array or
     *  key is in dict, otherwise null value will be returned.
     */
    Node& operator[] (size_t index);
    const Node& operator[] (size_t index) const { return const_cast<Node*>(this)->operator[](index); }

public: // dict
    DictIterator memberBegin();
    ConstDictIterator memberBegin() const;

    DictIterator memberEnd();
    ConstDictIterator memberEnd() const;

    DictIterator findMember(const char *key);
    DictIterator findMember(const std::string &key);
    DictIterator findMember(const Node &key);

    ConstDictIterator findMember(const char *key) const;
    ConstDictIterator findMember(const std::string &key) const;
    ConstDictIterator findMember(const Node &key) const;

    void eraseMember(DictIterator it);

    // 以下对dict操作的接口，key都支持路径形式，方便递归操作

    /** 支持路径形式的key，递归查找子结点
     *  @param keyPath   key以'/'分割。例如: 'a/b/c'
     *  @param keyLength key字符串的长度
     *  @return 如果未找到子节点，或者路径上的节点不是字典类型，则返回空节点的引用；否则，正常返回子节点的引用。
     */
    const Node& findMemberByPath(const char *keyPath, size_t keyLength) const;
    bool hasMemberByPath(const char *keyPath, size_t keyLength) const;
    bool setMemberByPath(const char *keyPath, size_t keyLength, const Node &val);
    bool removeMemberByPath(const char *keyPath, size_t keyLength);

    bool hasMember(const char *key) const { return hasMemberByPath(key, strlen(key)); }
    bool hasMember(const std::string &key) const { return hasMemberByPath(key.c_str(), key.size()); }
    bool hasMember(const Node &key) const;

    void setMember(const char *key, const Node &val) { setMemberByPath(key, strlen(key), val); }
    void setMember(const std::string &key, const Node &val) { setMemberByPath(key.c_str(), key.size(), val); }
    void setMember(const Node &key, const Node &val);

    bool removeMember(const char *key) { return removeMemberByPath(key, strlen(key)); }
    bool removeMember(const std::string &key) { return removeMemberByPath(key.c_str(), key.size()); }
    bool removeMember(const Node &key);

    // when the key was not found, null value will be returned.
    const Node& operator[] (const char *key) const;
    const Node& operator[] (const std::string &key) const;
    const Node& operator[] (const Node &key) const;

    template <typename T>
    T getMember(const char *key, T defaultValue = T()) const;

    void getKeys(std::vector<Node> &output, bool sort = false) const;
    void getValues(std::vector<Node> &output) const;
    void getMembers(std::vector<NodePair> &output, bool sort = false) const;

public:
    static inline bool compareMember(const NodePair &a, const NodePair &b) { return a.first < b.first; }

    static Node s_null;

private:

    void safeRelease();

    Node createTempKey(const char *str) const;
    Node createTempKey(const std::string &str) const;

    struct Value
    {
        union
        {
            bool        b;
            Integer     i;
            UInteger    u;
            Float       f;
            IObjectValue* p;
            StringValue*  ps;
            ArrayValue* pa;
            DictValue*  pd;
        };
    };

    ValueType   type_;
    Value       value_;
};

class ArrayValue : public IObjectValue
{
public:
    Array imp;

    ArrayValue(IAllocator *allocator)
        : IObjectValue(allocator)
    {}

    ValueType getType() const override { return T_ARRAY; }
    
    IObjectValue* clone() const override
    {
        ArrayValue *ret = allocator_->createArray(imp.size());
        ret->imp = imp;
        return ret;
    }

    IObjectValue* deepClone() const override;

    Array::iterator find(const Node &value);

    void remove(const Node &value)
    {
        auto it = find(value);
        if (it != imp.end())
        {
            imp.erase(it);
        }
    }
};

class DictValue : public IObjectValue
{
public:
    Dict imp;

    DictValue(IAllocator *allocator)
        : IObjectValue(allocator)
    {}

    bool remove(const Node &value)
    {
        auto it = imp.find(value);
        if (it != imp.end())
        {
            imp.erase(it);
            return true;
        }
        return false;
    }

    ValueType getType() const override { return T_DICT; }

    IObjectValue* clone() const override
    {
        DictValue *ret = allocator_->createDict(imp.size());
        ret->imp = imp;
        return ret;
    }

    IObjectValue* deepClone() const override;
};

inline Node& nullValue()
{
    Node::s_null.setNull();
    return Node::s_null;
}

// convert value to node

inline void toNode(Node &node, bool value)      { node.setBool(value); }

inline void toNode(Node &node, int8_t value)    { node.setInteger(value); }
inline void toNode(Node &node, uint8_t value)   { node.setUInteger(value); }
inline void toNode(Node &node, int16_t value)   { node.setInteger(value); }
inline void toNode(Node &node, uint16_t value)  { node.setUInteger(value); }
inline void toNode(Node &node, int32_t value)   { node.setInteger(value); }
inline void toNode(Node &node, uint32_t value)  { node.setUInteger(value); }
inline void toNode(Node &node, float value)     { node.setFloat(value); }

#if SJ_USE_LARGE_NUMBER
inline void toNode(Node &node, int64_t value)   { node.setInteger(value); }
inline void toNode(Node &node, uint64_t value)  { node.setUInteger(value); }
inline void toNode(Node &node, double value)    { node.setFloat(value); }
#else
inline void toNode(Node &node, int64_t value)   { node.setInteger(static_cast<Integer>(value)); }
inline void toNode(Node &node, uint64_t value)  { node.setUInteger(static_cast<UInteger>(value)); }
inline void toNode(Node &node, double value)    { node.setFloat(static_cast<Float>(value)); }
#endif

inline void toNode(Node &node, const char *value){ node.setString(value); }
inline void toNode(Node &node, const IObjectValue *value){ node.setObject(value); }
inline void toNode(Node &node, const std::string &value){ node.setString(value.c_str(), value.size()); }
inline void toNode(Node &node, const Node &value) { node = value; }

// parse value from node

inline void fromNode(bool &value, const Node &node)     { value = node.asBool(); }
inline void fromNode(int8_t &value, const Node &node)   { value = static_cast<int8_t>(node.asInteger()); }
inline void fromNode(uint8_t &value, const Node &node)  { value = static_cast<uint8_t>(node.asUInteger()); }
inline void fromNode(int16_t &value, const Node &node)  { value = static_cast<int16_t>(node.asInteger()); }
inline void fromNode(uint16_t &value, const Node &node) { value = static_cast<uint16_t>(node.asUInteger()); }
inline void fromNode(int32_t &value, const Node &node)  { value = static_cast<int32_t>(node.asInteger()); }
inline void fromNode(uint32_t &value, const Node &node) { value = static_cast<uint32_t>(node.asUInteger()); }
inline void fromNode(int64_t &value, const Node &node)  { value = static_cast<int64_t>(node.asInteger()); }
inline void fromNode(uint64_t &value, const Node &node) { value = static_cast<uint64_t>(node.asUInteger()); }
inline void fromNode(float &value, const Node &node)    { value = static_cast<float>(node.asFloat()); }
inline void fromNode(double &value, const Node &node)   { value = static_cast<double>(node.asFloat()); }
inline void fromNode(Node &value, const Node &node) { value = node; }

inline void fromNode(std::string &value, const Node &node)
{
    if (node.isString())
    {
        node.rawString()->to(value);
    }
    else
    {
        value.clear();
    }
}

// construct

inline Node::Node(const Node &other)
    : value_(other.value_)
    , type_(other.type_)
{
    if (isPointer())
    {
        value_.p->retain();
    }
}

inline Node::Node(Node &&other)
    : value_(other.value_)
    , type_(other.type_)
{
    other.type_ = T_NULL;
}

inline Node::Node(const char *str, size_t size, IAllocator *allocator)
    : type_(T_NULL)
{
    setString(str, size, allocator);
}

inline Node::Node(const std::string &str, IAllocator *allocator)
    : type_(T_NULL)
{
    setString(str.c_str(), str.size(), allocator);
}

template <typename T>
Node::Node(const T &value)
    : type_(T_NULL)
{
    toNode(*this, value);
}

inline void Node::safeRelease()
{
    if (isPointer())
    {
        value_.p->release();
    }
}

/////////////////////////////////////////////////////////////
/// conver value to json
/////////////////////////////////////////////////////////////

inline void Node::setNull()
{
    safeRelease();
    type_ = T_NULL;
    value_.p = 0;
}

inline void Node::setBool(bool v)
{
    safeRelease();
    type_ = T_BOOL;
    value_.b = v;
}

inline void Node::setInteger(Integer v)
{
    safeRelease();
    type_ = T_INT;
    value_.i = v;
}

inline void Node::setUInteger(UInteger v)
{
    safeRelease();
    type_ = T_INT;
    value_.u = v;
}

inline void Node::setFloat(Float v)
{
    safeRelease();
    type_ = T_FLOAT;
    value_.f = v;
}

template <typename T>
const Node& Node::operator = (const T& value)
{
    toNode(*this, value);
    return *this;
}

/////////////////////////////////////////////////////////////
/// convert json to value safely
/////////////////////////////////////////////////////////////
template <typename T>
inline T Node::as() const
{
    T ret;
    fromNode(ret, *this);
    return ret;
}

inline bool Node::asBool() const
{
    return isBool() ? value_.b : false;
}

inline Integer Node::asInteger() const
{
    if (isInt()) return value_.i;
    if (isFloat()) return static_cast<Integer>(value_.f);
    return 0;
}

inline UInteger Node::asUInteger() const
{
    if (isInt()) return value_.u;
    if (isFloat()) return static_cast<UInteger>(static_cast<Integer>(value_.f));
    return 0;
}

inline Float Node::asFloat() const
{
    if (isFloat()) return value_.f;
    if (isInt()) return (Float)value_.i;
    return (Float)0;
}

inline const char* Node::asCString() const
{
    return isString() ? rawString()->data() : "";
}

inline StringValue* Node::asString() const
{
    return isString() ? value_.ps : nullptr;
}

inline Array* Node::asArray() const
{
    return isArray() ? rawArray() : nullptr;
}

inline Dict* Node::asDict() const
{
    return isDict() ? rawDict() : nullptr;
}

/////////////////////////////////////////////////////////////
// convert json to value unsafe
/////////////////////////////////////////////////////////////
inline bool Node::rawBool() const
{
    return value_.b;
}

inline Integer Node::rawInteger() const
{
    return value_.i;
}

inline UInteger Node::rawUInteger() const
{
    return value_.u;
}

inline Float Node::rawFloat() const
{
    return value_.f;
}

inline const char* Node::rawCString() const
{
    return rawString()->data();
}


inline StringValue* Node::rawString() const
{
    SJ_ASSERT(isString());
    return value_.ps;
}

inline Array* Node::rawArray() const
{
    SJ_ASSERT(isArray());
    return &value_.pa->imp;
}

inline Dict* Node::rawDict() const
{
    SJ_ASSERT(isDict());
    return &value_.pd->imp;
}



inline StringValue* Node::rawString()
{
    SJ_ASSERT(isString());
    return value_.ps;
}

inline Array* Node::rawArray()
{
    SJ_ASSERT(isArray());
    return &value_.pa->imp;
}

inline Dict* Node::rawDict()
{
    SJ_ASSERT(isDict());
    return &value_.pd->imp;
}


inline StringValue& Node::refString()
{
    SJ_ASSERT(isString());
    return *(value_.ps);
}

inline Array& Node::refArray()
{
    SJ_ASSERT(isArray());
    return value_.pa->imp;
}

inline Dict& Node::refDict()
{
    SJ_ASSERT(isDict());
    return value_.pd->imp;
}


inline const StringValue& Node::refString() const
{
    SJ_ASSERT(isString());
    return *(value_.ps);
}

inline const Array& Node::refArray() const
{
    SJ_ASSERT(isArray());
    return value_.pa->imp;
}

inline const Dict& Node::refDict() const
{
    SJ_ASSERT(isDict());
    return value_.pd->imp;
}

inline IAllocator* Node::getAllocator() const
{
    if (isPointer())
    {
        return const_cast<IAllocator*>(value_.p->getAllocator());
    }
    return nullptr;
}

/////////////////////////////////////////////////////////////
/// array
/////////////////////////////////////////////////////////////
inline ArrayIterator Node::begin()
{
    return refArray().begin();
}

inline ConstArrayIterator Node::begin() const
{
    return refArray().begin();
}

inline ArrayIterator Node::end()
{
    return refArray().end();
}

inline ConstArrayIterator Node::end() const
{
    return refArray().end();
}

inline void Node::resize(size_t size)
{
    refArray().resize(size);
}

inline Node& Node::front()
{
    return refArray().front();
}

inline const Node& Node::front() const
{
    return refArray().front();
}

inline Node& Node::back()
{
    return refArray().back();
}

inline const Node& Node::back() const
{
    return refArray().back();
}

inline void Node::pushBack(const Node &node)
{
    refArray().push_back(node);
}

inline void Node::popBack()
{
    refArray().pop_back();
}

inline ArrayIterator Node::find(const Node &node)
{
    SJ_ASSERT(isArray());
    return value_.pa->find(node);
}

inline ConstArrayIterator Node::find(const Node &node) const
{
    SJ_ASSERT(isArray());
    return const_cast<ArrayValue*>(value_.pa)->find(node);
}

inline void Node::insert(ArrayIterator it, const Node &node)
{
    refArray().insert(it, node);
}

inline void Node::erase(ArrayIterator it)
{
    refArray().erase(it);
}

inline void Node::remove(const Node &node)
{
    SJ_ASSERT(isArray());
    value_.pa->remove(node);
}

/////////////////////////////////////////////////////////////
/// dict
/////////////////////////////////////////////////////////////
inline DictIterator Node::memberBegin()
{
    return refDict().begin();
}

inline ConstDictIterator Node::memberBegin() const
{
    return refDict().begin();
}

inline DictIterator Node::memberEnd()
{
    return refDict().end();
}

inline ConstDictIterator Node::memberEnd() const
{
    return refDict().end();
}

inline Node Node::createTempKey(const char *str) const
{
    return Node(value_.pd->getAllocator()->createString(str, strlen(str), BT_NOT_CARE));
}

inline Node Node::createTempKey(const std::string &str) const
{
    return Node(value_.pd->getAllocator()->createString(str.c_str(), str.size(), BT_NOT_CARE));
}

inline DictIterator Node::findMember(const char *key)
{
    return refDict().find(createTempKey(key));
}

inline DictIterator Node::findMember(const std::string &key)
{
    return refDict().find(createTempKey(key));
}

inline DictIterator Node::findMember(const Node &key)
{
    return refDict().find(key);
}

inline ConstDictIterator Node::findMember(const char *key) const
{
    return refDict().find(createTempKey(key));
}

inline ConstDictIterator Node::findMember(const std::string &key) const
{
    return refDict().find(createTempKey(key));
}

inline ConstDictIterator Node::findMember(const Node &key) const
{
    return refDict().find(key);
}

inline void Node::eraseMember(DictIterator it)
{
    refDict().erase(it);
}

template <typename T>
T Node::getMember(const char *key, T defaultValue) const
{
    const Node &node = findMemberByPath(key, strlen(key));
    if (!node.isNull())
    {
        return node.as<T>();
    }
    return defaultValue;
}

NS_SMARTJSON_END

namespace std
{
    inline size_t hash<NS_SMARTJSON Node>::operator()(const NS_SMARTJSON Node &node) const noexcept
    {
        return node.getHash();
    }
}
