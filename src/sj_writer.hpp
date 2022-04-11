#pragma once
#include <iostream>
#include "sj_node.hpp"

NS_SMARTJSON_BEGIN

class Node;

class Writer
{
public:
    Writer(const char *tab = "\t", const char *eol = "\n");
    
    void write(const Node &node, std::ostream &out);
    
    void writeNull(const Node &node, std::ostream &out);
    void writeBool(const Node &node, std::ostream &out);
    void writeInt(const Node &node, std::ostream &out);
    void writeFloat(const Node &node, std::ostream &out);
    void writeString(const Node &node, std::ostream &out);
    void writeNode(const Node &node, std::ostream &out, int depth);
    void writeArray(const Node &node, std::ostream &out, int depth);
    void writeDict(const Node &node, std::ostream &out, int depth);
    
private:
    const char*     tab_;
    const char*     eol_;
};
NS_SMARTJSON_END
