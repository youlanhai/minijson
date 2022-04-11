#pragma once

#ifndef JSON_USE_LARGE_NUMBER
#define JSON_USE_LARGE_NUMBER 1
#endif

#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__) ) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define SJ_PLATFORM_64BIT 1
#else
#define SJ_PLATFORM_64BIT 0
#endif

#ifndef JSON_ASSERT
#   if ((defined DEBUG) || defined(_DEBUG))
#include <cassert>
#       define JSON_ASSERT(EXP) assert(EXP)
#   else
#       define JSON_ASSERT(EXP)
#   endif
#endif //JSON_ASSERT

#define JSON_DISABLE_COPY_ASSIGN(CLASS) \
    CLASS(const CLASS&) = delete; \
    const CLASS& operator=(const CLASS&) = delete

#include <cfloat>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>

namespace mjson
{
#if JSON_USE_LARGE_NUMBER
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

    enum Type
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
        BT_NOT_CARE, // Object use the buffer freely, caller free buffer
        BT_MAKE_COPY, // Object should make a copy of the buffer
        BT_MANAGE, // Object should free this buffer
    };
    
    enum ResultCode
    {
        RC_OK,
        RC_END_OF_FILE,
        RC_OPEN_FILE_ERROR,
        RC_FILE_EMPTY,
        RC_INVALID_JSON,
        RC_INVALID_DICT,
        RC_INVALID_KEY,
        RC_INVALID_ARRAY,
        RC_INVALID_STRING,
        RC_INVALID_NUMBER,
        RC_INVALID_NULL,
        RC_INVALID_TRUE,
        RC_INVALID_FALSE,
        RC_INVALID_TYPE,
    };

    // predefine

    class Node;
    class IAllocator;

    class Object;
    class String;
    class ArrayValue;
    class DictValue;
}

namespace std
{
    template<>
    struct hash<mjson::Node>
    {
        typedef mjson::Node argument_type;
        typedef size_t result_type;
        _NODISCARD size_t operator()(const mjson::Node &node) const noexcept;
    };
}

namespace mjson
{
    typedef std::unordered_map<Node, Node> Dict;
    typedef std::vector<Node> Array;

    typedef Array::iterator ArrayIterator;
    typedef Array::const_iterator ConstArrayIterator;

    typedef Dict::iterator DictIterator;
    typedef Dict::const_iterator ConstDictIterator;

    /** 引用计数基类 */
    class Reference
    {
        JSON_DISABLE_COPY_ASSIGN(Reference);
    public:
        Reference() : refCount_(0) {}
        virtual ~Reference() {}

        long getRefCount() const { return refCount_; }
        void retain() { ++refCount_; }

        void release()
        {
            JSON_ASSERT(refCount_ > 0);
            if (--refCount_ <= 0)
            {
                destroyThis();
            }
        }

        virtual void destroyThis() { delete this; }

    private:
        long        refCount_;
    };

    class IAllocator : public Reference
    {
    public:
        IAllocator() = default;
        ~IAllocator() = default;
        
        virtual String* createString(const char *str, size_t size, BufferType type);
        virtual ArrayValue* createArray(size_t capacity);
        virtual DictValue* createDict(size_t capacity);
        
        virtual void freeObject(Object *p);
        
        static void setDefaultAllocator(IAllocator *p);
        static IAllocator* getDefaultAllocator(){ return s_pDefault; }
        
    private:
        static IAllocator *s_pDefault;
    };

    class Object : public Reference
    {
        JSON_DISABLE_COPY_ASSIGN(Object);
    public:
        explicit Object(IAllocator *allocator);
        ~Object();
        
        virtual void    destroyThis() override;
        virtual Type    type() const = 0;
        virtual Object* clone() const = 0;
        virtual Object* deepClone() const = 0;
        
        inline IAllocator* getAllocator(){ return allocator_; }
        
    protected:
        IAllocator*     allocator_;
    };

    class String : public Object
    {
        JSON_DISABLE_COPY_ASSIGN(String);
    public:
        String(const char *str, size_t size, IAllocator *allocator);
        ~String() = default;
        
        size_t size() const { return size_; }

        const char* data() const { return str_; }

        Type type() const { return T_STRING; }

        int compare(const char *str) const
        {
            return compare(str, strlen(str));
        }

        int compare(const char *str, size_t length) const
        {
            size_t minSize = size_ < length ? size_ : length;
            int ret = memcmp(str_, str, minSize);
            if (ret == 0)
            {
                return (int)size_ - (int)length;
            }
            return ret;
        }

        int compare(const String *p) const
        {
            return compare(p->str_, p->size_);
        }

        int compare(const std::string &str) const
        {
            return compare(str.c_str(), str.size());
        }
        
        //string is constant, deosn't need clone.
        Object* clone() const { return const_cast<String*>(this); }
        Object* deepClone() const {return clone(); }

        size_t getHash() const
        {
            if (hash_ == (size_t)-1)
            {
                hash_ = computeHash();
            }
            return hash_;
        }

        size_t computeHash() const;
        
    private:
        const char*     str_;
        size_t          size_;
        mutable size_t  hash_;
        BufferType      bufferType_;
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

        Node(Object *p);

        Node(const char *str, size_t size, IAllocator *allocator = nullptr);
        Node(const std::string &value, IAllocator *allocator = 0);

        explicit Node(Type type, IAllocator *allocator = 0);

        void safeRelease();

        Type getType() const { return type_; }

        bool isNull() const { return type_ == T_NULL; }
        bool isBool() const { return type_ == T_BOOL; }
        bool isInt() const { return type_ == T_INT; }
        bool isFloat() const { return type_ == T_FLOAT; }
        bool isString() const { return type_ == T_STRING; }
        bool isArray() const { return type_ == T_ARRAY; }
        bool isDict() const { return type_ == T_DICT; }
        bool isNumber() const { return type_ == T_INT || type_ == T_FLOAT; }
        bool isPointer() const { return type_ > T_POINTER; }

        bool        asBool()    const;
        Integer     asInteger() const;
        UInteger    asUInteger()const;
        Float       asFloat()   const;
        String*     asString()  const;
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
        String*     rawString() const;
        Array*      rawArray() const;
        Dict*       rawDict() const;

        String*     rawString();
        Array*      rawArray();
        Dict*       rawDict();

        const String&   refString() const;
        const Array&    refArray() const;
        const Dict&     refDict() const;

        String&     refString();
        Array&      refArray();
        Dict&       refDict();

        //}


        void        setNull();
        void        setBool(bool v);
        void        setInteger(Integer v);
        void        setUInteger(UInteger v);
        void        setFloat(Float v);
        void        setObject(const Object *p);
        void        setString(const char *str, size_t size = 0, IAllocator *allocator = 0);
        Array*      setArray(IAllocator *allocator = 0);
        Dict*       setDict(IAllocator *allocator = 0);

        template <typename T>
        const Node& operator = (const T& value);
        const Node& operator = (const Node &value);
        const Node& operator = (Node &&value);

        bool operator == (const Node &value) const;
        bool operator != (const Node &value) const;
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
        const Node& operator[] (size_t index) const;

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

        bool hasMember(const char *key) const;
        bool hasMember(const std::string &key) const;
        bool hasMember(const Node &key) const;

        void setMember(const char *key, const Node &val);
        void setMember(const std::string &key, const Node &val);
        void setMember(const Node &key, const Node &val);

        void eraseMember(DictIterator it);

        void removeMember(const char *key);
        void removeMember(const std::string &key);
        void removeMember(const Node &key);

        // when the key was not found, null value will be returned.
        const Node& operator[] (const char *key) const;
        const Node& operator[] (const std::string &key) const;
        const Node& operator[] (const Node &key) const;

    public:
         static Node s_null;

    private:
        struct Value
        {
            union
            {
                bool        b;
                Integer     i;
                UInteger    u;
                Float       f;
                Object*     p;
                String*     ps;
                ArrayValue* pa;
                DictValue*  pd;
            };
        };

        Type        type_;
        Value       value_;
    };

    class ArrayValue : public Object
    {
    public:
        Array imp;

        ArrayValue(IAllocator *allocator)
            : Object(allocator)
        {}

        Type type() const override { return T_ARRAY; }
        
        Object* clone() const override
        {
            ArrayValue *ret = allocator_->createArray(imp.size());
            ret->imp = imp;
            return ret;
        }

        Object* deepClone() const override;

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

    class DictValue : public Object
    {
    public:
        Dict imp;

        DictValue(IAllocator *allocator)
            : Object(allocator)
        {}

        void remove(const Node &value)
        {
            auto it = imp.find(value);
            if (it != imp.end())
            {
                imp.erase(it);
            }
        }

        Type type() const override { return T_DICT; }

        Object* clone() const override
        {
            DictValue *ret = allocator_->createDict(imp.size());
            ret->imp = imp;
            return ret;
        }

        Object* deepClone() const override;
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

#if JSON_USE_LARGE_NUMBER
    inline void toNode(Node &node, int64_t value)   { node.setInteger(value); }
    inline void toNode(Node &node, uint64_t value)  { node.setUInteger(value); }
    inline void toNode(Node &node, double value)    { node.setFloat(value); }
#else
    inline void toNode(Node &node, int64_t value)   { node.setInteger(static_cast<Integer>(value)); }
    inline void toNode(Node &node, uint64_t value)  { node.setUInteger(static_cast<UInteger>(value)); }
    inline void toNode(Node &node, double value)    { node.setFloat(static_cast<Float>(value)); }
#endif

    inline void toNode(Node &node, const char *value){ node.setString(value); }
    inline void toNode(Node &node, const Object *value){ node.setObject(value); }
    inline void toNode(Node &node, const std::string &value){ node.setString(value.c_str(), value.size()); }

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

    inline void fromNode(std::string &value, const Node &node)
    {
        if (node.isString())
        {
            String *p = node.rawString();
            value.assign(p->data(), p->size());
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

    inline Node::Node(Object *p)
    {
        if (p != nullptr)
        {
            p->retain();
            type_ = p->type();
        }
        else
        {
            type_ = T_NULL;
        }
        value_.p = p;
    }

    inline Node::Node(const char *str, size_t size, IAllocator *allocator)
        : type_(T_NULL)
    {
        setString(str, size, allocator);
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


    inline const Node& Node::operator = (const Node &other)
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

    inline const Node& Node::operator = (Node &&other)
    {
        safeRelease();
        type_ = other.type_;
        value_ = other.value_;
        other.type_ = T_NULL;
        return *this;
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

    inline String* Node::asString() const
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
        JSON_ASSERT(isString());
        return rawString()->data();
    }


    inline String* Node::rawString() const
    {
        JSON_ASSERT(isString());
        return value_.ps;
    }

    inline Array* Node::rawArray() const
    {
        JSON_ASSERT(isArray());
        return &value_.pa->imp;
    }

    inline Dict* Node::rawDict() const
    {
        JSON_ASSERT(isDict());
        return &value_.pd->imp;
    }



    inline String* Node::rawString()
    {
        JSON_ASSERT(isString());
        return value_.ps;
    }

    inline Array* Node::rawArray()
    {
        JSON_ASSERT(isArray());
        return &value_.pa->imp;
    }

    inline Dict* Node::rawDict()
    {
        JSON_ASSERT(isDict());
        return &value_.pd->imp;
    }


    inline String& Node::refString()
    {
        JSON_ASSERT(isString());
        return *(value_.ps);
    }

    inline Array& Node::refArray()
    {
        JSON_ASSERT(isArray());
        return value_.pa->imp;
    }

    inline Dict& Node::refDict()
    {
        JSON_ASSERT(isDict());
        return value_.pd->imp;
    }


    inline const String& Node::refString() const
    {
        JSON_ASSERT(isString());
        return *(value_.ps);
    }

    inline const Array& Node::refArray() const
    {
        JSON_ASSERT(isArray());
        return value_.pa->imp;
    }

    inline const Dict& Node::refDict() const
    {
        JSON_ASSERT(isDict());
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
    /// 
    /////////////////////////////////////////////////////////////
    inline size_t Node::size() const
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

    inline size_t Node::capacity() const
    {
        if (isArray())
        {
            return refArray().capacity();
        }
        return 0;
    }

    inline void Node::reserve(size_t capacity)
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

    inline void Node::clear()
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

    inline bool Node::operator != (const Node &value) const
    {
        return !(*this == value);
    }

    inline Node Node::clone() const
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

    inline Node Node::deepClone() const
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

    inline const Node& Node::operator[] (const char *key) const
    {
        if (isDict())
        {
            ConstDictIterator it = findMember(key);
            if (it != memberEnd())
            {
                return it->second;
            }
        }
        return nullValue();
    }

    inline const Node& Node::operator[] (const Node &key) const
    {
        if (isDict())
        {
            ConstDictIterator it = findMember(key);
            if (it != memberEnd())
            {
                return it->second;
            }
        }
        return nullValue();
    }

    /////////////////////////////////////////////////////////////
    /// array
    /////////////////////////////////////////////////////////////
    inline ArrayIterator Node::begin()
    {
        JSON_ASSERT(isArray());
        return refArray().begin();
    }

    inline ConstArrayIterator Node::begin() const
    {
        JSON_ASSERT(isArray());
        return refArray().begin();
    }

    inline ArrayIterator Node::end()
    {
        JSON_ASSERT(isArray());
        return refArray().end();
    }

    inline ConstArrayIterator Node::end() const
    {
        JSON_ASSERT(isArray());
        return refArray().end();
    }

    inline void Node::resize(size_t size)
    {
        JSON_ASSERT(isArray());
        refArray().resize(size);
    }

    inline Node& Node::front()
    {
        JSON_ASSERT(isArray());
        return refArray().front();
    }

    inline const Node& Node::front() const
    {
        JSON_ASSERT(isArray());
        return refArray().front();
    }

    inline Node& Node::back()
    {
        JSON_ASSERT(isArray());
        return refArray().back();
    }

    inline const Node& Node::back() const
    {
        JSON_ASSERT(isArray());
        return refArray().back();
    }

    inline void Node::pushBack(const Node &node)
    {
        JSON_ASSERT(isArray());
        refArray().push_back(node);
    }

    inline void Node::popBack()
    {
        JSON_ASSERT(isArray());
        refArray().pop_back();
    }

    inline ArrayIterator Node::find(const Node &node)
    {
        JSON_ASSERT(isArray());
        return value_.pa->find(node);
    }

    inline ConstArrayIterator Node::find(const Node &node) const
    {
        JSON_ASSERT(isArray());
        return const_cast<ArrayValue*>(value_.pa)->find(node);
    }

    inline void Node::insert(ArrayIterator it, const Node &node)
    {
        JSON_ASSERT(isArray());
        refArray().insert(it, node);
    }

    inline void Node::erase(ArrayIterator it)
    {
        JSON_ASSERT(isArray());
        refArray().erase(it);
    }

    inline void Node::remove(const Node &node)
    {
        JSON_ASSERT(isArray());
        value_.pa->remove(node);
    }

    inline const Node& Node::operator[] (size_t index) const
    {
        return const_cast<Node*>(this)->operator[](index);
    }

    /////////////////////////////////////////////////////////////
    /// dict
    /////////////////////////////////////////////////////////////
    inline DictIterator Node::memberBegin()
    {
        JSON_ASSERT(isDict());
        return refDict().begin();
    }

    inline ConstDictIterator Node::memberBegin() const
    {
        JSON_ASSERT(isDict());
        return refDict().begin();
    }

    inline DictIterator Node::memberEnd()
    {
        JSON_ASSERT(isDict());
        return refDict().end();
    }

    inline ConstDictIterator Node::memberEnd() const
    {
        JSON_ASSERT(isDict());
        return refDict().end();
    }

    inline DictIterator Node::findMember(const char *key)
    {
        JSON_ASSERT(isDict());
        String *s = value_.pd->getAllocator()->createString(key, strlen(key), BT_NOT_CARE);
        return refDict().find(Node(s));
    }

    inline DictIterator Node::findMember(const Node &key)
    {
        JSON_ASSERT(isDict());
        return refDict().find(key);
    }

    inline ConstDictIterator Node::findMember(const char *key) const
    {
        JSON_ASSERT(isDict());
        String *s = value_.pd->getAllocator()->createString(key, strlen(key), BT_NOT_CARE);
        return refDict().find(Node(s));
    }

    inline ConstDictIterator Node::findMember(const Node &key) const
    {
        JSON_ASSERT(isDict());
        return refDict().find(key);
    }

    inline bool Node::hasMember(const char *key) const
    {
        return findMember(key) != refDict().end();
    }

    inline bool Node::hasMember(const Node &key) const
    {
        return findMember(key) != refDict().end();
    }

    inline void Node::setMember(const char *key, const Node &val)
    {
        JSON_ASSERT(isDict());
        String *s = value_.pd->getAllocator()->createString(key, strlen(key), BT_MAKE_COPY);
        refDict()[Node(s)] = val;
    }

    inline void Node::setMember(const Node &key, const Node &val)
    {
        JSON_ASSERT(isDict());
        refDict()[key] = val;
    }

    inline void Node::eraseMember(DictIterator it)
    {
        JSON_ASSERT(isDict());
        refDict().erase(it);
    }

    inline void Node::removeMember(const char *key)
    {
        JSON_ASSERT(isDict());
        String *s = value_.pd->getAllocator()->createString(key, strlen(key), BT_NOT_CARE);
        value_.pd->remove(Node(s));
    }

    inline void Node::removeMember(const Node &key)
    {
        JSON_ASSERT(isDict());
        value_.pd->remove(key);
    }

    /////////////////////////////////////////////////////////////
    /// std::string
    /////////////////////////////////////////////////////////////
    inline Node::Node(const std::string &value, IAllocator *allocator)
        : type_(T_NULL)
    {
        setString(value.c_str(), value.size(), allocator);
    }

    inline const Node& Node::operator[] (const std::string &key) const
    {
        return operator[](key.c_str());
    }

    inline DictIterator Node::findMember(const std::string &key)
    {
        JSON_ASSERT(isDict());
        String *s = value_.pd->getAllocator()->createString(key.c_str(), key.size(), BT_NOT_CARE);
        return refDict().find(Node(s));
    }

    inline ConstDictIterator Node::findMember(const std::string &key) const
    {
        JSON_ASSERT(isDict());
        String *s = value_.pd->getAllocator()->createString(key.c_str(), key.size(), BT_NOT_CARE);
        return refDict().find(Node(s));
    }

    inline bool Node::hasMember(const std::string &key) const
    {
        return findMember(key) != refDict().end();
    }

    inline void Node::setMember(const std::string &key, const Node &val)
    {
        JSON_ASSERT(isDict());
        // copy string buffer
        String *s = value_.pd->getAllocator()->createString(key.c_str(), key.size(), BT_MAKE_COPY);
        refDict()[Node(s)] = val;
    }

    inline void Node::removeMember(const std::string &key)
    {
        JSON_ASSERT(isDict());
        String *s = value_.pd->getAllocator()->createString(key.c_str(), key.size(), BT_NOT_CARE);
        value_.pd->remove(Node(s));
    }
}


namespace std
{
    inline size_t hash<mjson::Node>::operator()(const mjson::Node &node) const noexcept
    {
        return node.getHash();
    }
}
