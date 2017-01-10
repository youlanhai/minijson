#ifndef SMARTJSON_VALUE_SETTER_HPP
#define SMARTJSON_VALUE_SETTER_HPP

namespace mjson
{
    class ValueSetter
    {
        Node    key_;
        Node    node_;

    public:
        explicit ValueSetter(const Node &key)
        : key_(key)
        {}

        template<typename T>
        ValueSetter& operator = (const T &value)
        {
            node_.setMember(key_, value);
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
    };
}

#endif //SMARTJSON_VALUE_SETTER_HPP
