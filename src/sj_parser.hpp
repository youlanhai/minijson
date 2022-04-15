#pragma once
#include "sj_node.hpp"
#include <iostream>

NS_SMARTJSON_BEGIN

class IParser
{
    SJ_DISABLE_COPY_ASSIGN(IParser);
public:
    explicit IParser(IAllocator *allocator);
    virtual ~IParser();

    bool parseFromFile(const char *fileName);
    bool parseFromFile(const std::string& fileName);

    bool parseFromData(const char *str, size_t length);
    bool parseFromString(const std::string &str);

    bool parse(std::istream &stream);

    Node getRoot() const { return root_; }
    int getErrorCode() const { return errorCode_; }

protected:
    virtual bool doParse() = 0;

    bool onError(int code);

protected:

    IAllocator*     allocator_;
    std::istream*   stream_ = nullptr;
    Node            root_;
    int 			errorCode_ = RC_OK;
    bool            isBinaryFile_ = false;
};

class IWriter
{
public:
    IWriter() = default;
    virtual ~IWriter() = default;

    bool writeToFile(const Node &node, const char *fileName);
    bool writeToFile(const Node &node, const std::string& fileName);

    bool write(const Node &node, std::ostream &out);
    std::string toString(const Node &node);

    int getErrorCode() const { return errorCode_; }

protected:

    bool onError(int code);

    virtual void onWrite(const Node &node) = 0;

public:
    const char*     tab_ = "\t";

    const char*     eol_ = "\n";

    /** 是否对字典key进行排序 */
    bool            sortKey_ = false;

    bool            isBinaryFile_ = false;

protected:
    std::ostream*   stream_ = nullptr;
    int 			errorCode_ = RC_OK;
};

/** json parser.
 *  bnf: http://www.json.org/json-zh.html
 *  NOTICE: The format '\uxxxx' doesn't supported.
 */
class Parser : public IParser
{
    SJ_DISABLE_COPY_ASSIGN(Parser);
public:
    explicit Parser(IAllocator *allocator = nullptr);
    
    int getLine() const { return line_; }
    int getColumn() const { return column_; }

private:
    bool doParse() override;

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

private:
    std::vector<char> stringBuffer_;
    int             line_;
    int             column_;
    int             nextToken_;
};

class Writer : public IWriter
{
public:
    Writer(const char *tab = "\t", const char *eol = "\n");
    
    // 以下接口，外部也可以直接使用

    void writeNull(const Node &node, std::ostream &out);
    void writeBool(const Node &node, std::ostream &out);
    void writeInt(const Node &node, std::ostream &out);
    void writeFloat(const Node &node, std::ostream &out);
    void writeString(const Node &node, std::ostream &out);
    void writeNode(const Node &node, std::ostream &out, int depth);
    void writeArray(const Node &node, std::ostream &out, int depth);
    void writeDict(const Node &node, std::ostream &out, int depth);
    
protected:

    void onWrite(const Node &node) override;

public:
    /** 字典元素分隔符 */
    const char*     seperator_ = " : ";

    /** 是否在数组和字典尾部元素后增加逗号。
     *  加上尾部逗号，可以减少版本控制冲突。但是别的json工具可能会读取失败。
     */
    bool            endComma_ = false;
};

typedef Parser JsonParser;
typedef Writer JsonWriter;

std::ostream& operator << (std::ostream & stream, const Node &v);

NS_SMARTJSON_END
