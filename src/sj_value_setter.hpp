#ifndef SMARTJSON_VALUE_SETTER_HPP
#define SMARTJSON_VALUE_SETTER_HPP

namespace mjson
{
    class ValueSetter
    {
        Node    key_;
        Node    node_;

    public:
        ValueSetter(Node &node, const Node &key)
        : node_(node)
        , key_(key)
        {}

        template<typename T>
        ValueSetter& operator = (const T &value)
        {
            node_.setMember(key_, value);
            return *this;
        }
        
        template<typename T>
        bool operator == (const T &value) const
        {
            return get() == value;
        }
        
        template<typename T>
        bool operator != (const T &value) const
        {
            return get() != value;
        }

        const Node& get() const
        {
            return node_[key_];
        }

        friend bool operator == (const Node &value, const ValueSetter &val);
    };

    inline bool operator == (const Node &value, const ValueSetter &val)
    {
        return value == val.get();
    }

    inline bool operator != (const Node &value, const ValueSetter &val)
    {
        return value != val.get();
    }
}

#endif //SMARTJSON_VALUE_SETTER_HPP
