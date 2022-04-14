#pragma once
#include "sj_node.hpp"
#include <iostream>

NS_SMARTJSON_BEGIN

/** json parser.
 *  bnf: http://www.json.org/json-zh.html
 *  NOTICE: The format '\uxxxx' doesn't supported.
 */
class Parser
{
public:
    explicit Parser(IAllocator *allocator = nullptr);
    ~Parser();
    
    // return one of `ResultCode`.
    bool parseFromFile(const char *fileName);
    bool parseFromData(const char *str, size_t length);
    bool parseFromStream(std::istream &stream);
    
    Node getRoot() const{ return root_; }
    int getErrorCode() const { return errorCode_; }
    int getLine() const { return line_; }
    int getColumn() const { return column_; }

private:
    char getChar();
    void ungetChar(char ch);
    char translateChar(char ch);

    int parseToken();
    int nextToken();
    int aheadToken();
    
    bool parseDict(Node &node);
    bool parseArray(Node &node);
    bool parseNumber(Node &node, char ch);
    bool parseString(Node &node);
    bool parseTrue(Node &node);
    bool parseFalse(Node &node);
    bool parseNull(Node &node);
    bool parseValue(Node &node);

    bool parseComment();
    bool parseLineComment();
    bool parseLongComment();
    
    bool parseUnicodeChar();

    bool onError(int code);
    
private:
    IAllocator*     allocator_;
    std::vector<char> stringBuffer_;
    
    Node            root_;
    std::istream*   stream_;
    int 			errorCode_;
    int             line_;
    int             column_;
    int             nextToken_;
};

class Writer
{
public:
    Writer(const char *tab = "\t", const char *eol = "\n");
    
    void write(const Node &node, std::ostream &out);
    
    // 以下接口，外部也可以直接使用

    void writeNull(const Node &node, std::ostream &out);
    void writeBool(const Node &node, std::ostream &out);
    void writeInt(const Node &node, std::ostream &out);
    void writeFloat(const Node &node, std::ostream &out);
    void writeString(const Node &node, std::ostream &out);
    void writeNode(const Node &node, std::ostream &out, int depth);
    void writeArray(const Node &node, std::ostream &out, int depth);
    void writeDict(const Node &node, std::ostream &out, int depth);
    
public:
    const char*     tab_;
    const char*     eol_;

    /** 字典元素分隔符 */
    const char*     seperator_ = " : ";

    /** 是否对字典key进行排序 */
    bool            sortKey_ = false;

    /** 是否在数组和字典尾部元素后增加逗号。
     *  加上尾部逗号，可以减少版本控制冲突。但是别的json工具可能会读取失败。
     */
    bool            endComma_ = false;
};

NS_SMARTJSON_END
