#ifndef MJSON_ITERATOR_H
#define MJSON_ITERATOR_H

#include "node.hpp"
#include "container/array.hpp"
#include "container/dict.hpp"

namespace mjson
{
    class ArrayIterator
    {
        Node &node_;
    public:
        typedef Array::iterator iterator;

        ArrayIterator(Node &node)
            : node_(node)
        {}

        iterator begin(){ return node_.isArray() ? node_.rawArray()->begin() : nullptr; }
        iterator end(){ return node_.isArray() ? node_.rawArray()->end() : nullptr; }
    };

    class ConstArrayIterator
    {
        const Node &node_;
    public:
        typedef Array::const_iterator iterator;

        ConstArrayIterator(const Node &node)
            : node_(node)
        {}

        iterator begin(){ return node_.isArray() ? node_.rawArray()->begin() : nullptr; }
        iterator end(){ return node_.isArray() ? node_.rawArray()->end() : nullptr; }
    };


    class DictIterator
    {
        Node &node_;
    public:
        typedef Dict::iterator  iterator;

        DictIterator(Node &node)
            : node_(node)
        {}

        iterator begin(){ return node_.isDict() ? node_.rawDict()->begin() : nullptr; }
        iterator end(){ return node_.isDict() ? node_.rawDict()->end() : nullptr; }
    };

    class ConstDictIterator
    {
        const Node &node_;
    public:
        typedef Dict::iterator  iterator;

        ConstDictIterator(const Node &node)
            : node_(node)
        {}

        iterator begin() const { return node_.isDict() ? node_.rawDict()->begin() : nullptr; }
        iterator end() const { return node_.isDict() ? node_.rawDict()->end() : nullptr; }
    };
}

#endif // MJSON_ITERATOR_H
