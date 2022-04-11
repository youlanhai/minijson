#include <iostream>

#include "smartjson.hpp"
#include "sj_binary_parser.hpp"
#include "sj_allocator_imp.hpp"

#include <string>
#include <cassert>
#include <cmath>
#include <fstream>

#define TEST_EQUAL(EXP) testEqual(EXP, #EXP, __LINE__)
void testEqual(bool ret, const char *exp, int line)
{
    if(!ret)
    {
        std::cout << "TestFailed: (" << exp << "), line: " << line << std::endl;
        abort();
    }
}

bool almoseEqual(double a, double b, double epsilon = 0.00001)
{
    return std::abs(a - b) < epsilon;
}

void testString()
{
    std::cout << "test string..." << std::endl;
    
    smartjson::IAllocator *allocator = smartjson::IAllocator::getDefaultAllocator();
    
    std::string str("Hello World");
    smartjson::StringValue *p = allocator->createString(str.c_str(), str.size(), smartjson::BT_MAKE_COPY);
    p->retain();
    
    TEST_EQUAL(p->getRefCount() == 1);
    TEST_EQUAL(p->size() == str.size());
    TEST_EQUAL(str == p->data());
    TEST_EQUAL(p->compare(str.c_str()) == 0);
    
    smartjson::StringValue *p2 = (smartjson::StringValue*)p->clone();
    p2->retain();
    TEST_EQUAL(p->compare(p2) == 0);
    p2->release();
    
    smartjson::StringValue *p3 = (smartjson::StringValue*)p->deepClone();
    p3->retain();
    TEST_EQUAL(p->compare(p3) == 0);
    p3->release();
    
    p->release();
}


void testNode()
{
    std::cout << "test node..." << std::endl;
    
    smartjson::Node n0;
    TEST_EQUAL(n0.isNull());
    TEST_EQUAL(n0 == smartjson::Node());
    
    smartjson::Node n1(true);
    TEST_EQUAL(n1.isBool());
    TEST_EQUAL(n1.asBool() == true);
    
    smartjson::Node n2(1234567890);
    TEST_EQUAL(n2.isInt());
    TEST_EQUAL(n2.isNumber());
    TEST_EQUAL(n2.asInteger() == 1234567890);
    
    smartjson::Node n3(1234.5f);
    TEST_EQUAL(n3.isFloat());
    TEST_EQUAL(n3.isNumber());
    TEST_EQUAL(n3.asFloat() == 1234.5);
    
    smartjson::Node n4(1234.5);
    TEST_EQUAL(n4.isFloat());
    TEST_EQUAL(n4.isNumber());
    TEST_EQUAL(n4.asFloat() == 1234.5);
    TEST_EQUAL(n4 == n3);
    TEST_EQUAL(n4 != n2);
    TEST_EQUAL(n4 != n1);
    TEST_EQUAL(n4 != n0);
    
    smartjson::Node n5("Hello World");
    TEST_EQUAL(n5.isString());
    TEST_EQUAL(n5.isPointer());
    TEST_EQUAL(n5.asString() != 0);
    TEST_EQUAL(strcmp(n5.asCString(), "Hello World") == 0);
    TEST_EQUAL(n5 != n0);
    TEST_EQUAL(n5 != n1);
    TEST_EQUAL(n5 != n2);
    TEST_EQUAL(n5 != n3);
    TEST_EQUAL(n5 != n4);
    TEST_EQUAL(n5 == n5);
    TEST_EQUAL(n5.size() == n5.asString()->size());
    
    n5 = n5;
    TEST_EQUAL(n5.asString()->getRefCount() == 1);
    
    smartjson::Node n6(n5);
    TEST_EQUAL(n6.isString());
    TEST_EQUAL(n6 == n5);
    TEST_EQUAL(n6.asString() == n5.asString());
    TEST_EQUAL(n6.asString()->getRefCount() == 2);
    
    n6 = n5;
    TEST_EQUAL(n6.isString());
    TEST_EQUAL(n6.asString()->getRefCount() == 2);
    TEST_EQUAL(n6.asString() == n5.asString());
    
    // test array
    smartjson::Node n7;
    n7.setArray();
    TEST_EQUAL(n7.isArray());
    n7.reserve(7);
    TEST_EQUAL(n7.capacity() == 7);
    TEST_EQUAL(n7.refArray().capacity() == 7);
    n7.pushBack(n0);
    n7.pushBack(n1);
    n7.pushBack(n2);
    n7.pushBack(n3);
    n7.pushBack(n4);
    n7.pushBack(n5);
    n7.pushBack(n6);
    TEST_EQUAL(n7.asArray()->size() == n7.size());
    TEST_EQUAL(n7.size() == 7);
    TEST_EQUAL(n7[0u] == n0);
    for(smartjson::ArrayIterator it = n7.begin();
        it != n7.end(); ++it)
    {
        TEST_EQUAL(*it != n7);
    }
    for(size_t i = 0; i < n7.size(); ++i)
    {
        TEST_EQUAL(n7[i] == n7.asArray()->at(i));
    }
    
    // test dict
    smartjson::Node n8;
    n8.setDict();
    TEST_EQUAL(n8.isDict());
    n8.setMember("0", n0);
    n8.setMember("1", n1);
    n8.setMember("2", n2);
    n8.setMember("3", n3);
    n8.setMember("4", n4);
    n8.setMember("5", n5);
    n8.setMember("6", n6);
    n8.setMember("7", n7);
    TEST_EQUAL(n8.size() == 8);
    TEST_EQUAL(n8.size() == n8.rawDict()->size());
    for(smartjson::DictIterator it = n8.memberBegin();
        it != n8.memberEnd(); ++it)
    {
        TEST_EQUAL(it->second == n8[it->first]);
    }
}

struct Vector2
{
    float x, y;
};

void fromNode(Vector2 &v, const smartjson::Node &node)
{
    v.x = node.getMember<float>("x");
    v.y = node.getMember<float>("y");
}

void testParser()
{
    std::cout << "test parser..." << std::endl;
    
    const char *json = "{\n"
    "\"name\"   : \"json\",\n"
    "\"age\"    : 20,\n"
    "\"weight\" : 60.5,\n"
    "\"i1\"     : 1234567890,\n"
    "\"i2\"     : -123456789,\n"
    "\"i3\"     : 0,\n"
    "\"f1\"     : 3.14e2,\n"
    "\"f2\"     : 3.14e-2,\n"
    "\"f3\"     : -0.3140e10,\n"
    "\"f4\"     : -0.314e-10,\n"
    "\"array\"  : [0, true, false, null, 123, -456, 3.14, \"hello\\n world!\"],\n"
    "\"pos\"    : {\"x\" : 100.55, \"y\" : 200.22}\n"
    "}";
    
    smartjson::Parser parser(new smartjson::MemoryPoolAllocator());
    bool ret = parser.parseFromData(json, strlen(json));
    std::cout << "parse result:" << parser.getErrorCode() << std::endl;
    TEST_EQUAL(ret == true);
    
    smartjson::Node root = parser.getRoot();
    const smartjson::Node &croot = root;
    TEST_EQUAL(croot["name"] == "json");
    TEST_EQUAL(croot["age"] == 20);
    TEST_EQUAL(croot["weight"] == 60.5);
    TEST_EQUAL(croot["i1"] == 1234567890);
    TEST_EQUAL(croot["i2"] == -123456789);
    TEST_EQUAL(croot["i3"] == 0);
    TEST_EQUAL(croot["f1"] == 3.14e2);
    TEST_EQUAL(almoseEqual(croot["f2"].asFloat(), 3.14e-2));
    TEST_EQUAL(almoseEqual(croot["f3"].asFloat(), -0.314e10));
    TEST_EQUAL(almoseEqual(croot["f4"].asFloat(), -0.314e-10));
    
    smartjson::Node array = root["array"];
    TEST_EQUAL(array.isArray());
    TEST_EQUAL(array[0u] == 0);
    TEST_EQUAL(array[1] == true);
    TEST_EQUAL(array[2] == false);
    TEST_EQUAL(array[3].isNull());
    TEST_EQUAL(array[4] == 123);
    TEST_EQUAL(array[5] == -456);
    TEST_EQUAL(array[6] == 3.14);
    TEST_EQUAL(array[7] == "hello\n world!");
    
    smartjson::Node pos = root["pos"];
    TEST_EQUAL(pos.isDict());
    TEST_EQUAL(pos["x"] == 100.55);
    TEST_EQUAL(pos["y"] == 200.22);

    Vector2 v2 = pos.as<Vector2>();
    TEST_EQUAL(almoseEqual(v2.x, 100.55));
    TEST_EQUAL(almoseEqual(v2.y, 200.22));

    Vector2 v2x = root.getMember<Vector2>("pos");
    TEST_EQUAL(almoseEqual(v2x.x, 100.55));
    TEST_EQUAL(almoseEqual(v2x.y, 200.22));
    
    std::cout << "print json:" << std::endl;
    smartjson::Writer writer;
    writer.write(root, std::cout);
}

void testBinaryParser()
{
    std::cout << "test binary parser ..." << std::endl;
    
    const char *fileName = "test_sheet.ab";
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL)
    {
        std::cout << "Failed open file " << fileName << std::endl;
        return;
    }
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *buffer = new char[length];
    fread(buffer, 1, length, fp);
    
    fclose(fp);
    fp = NULL;
    
    smartjson::BinaryParser parser;
    if(parser.parseFromData(buffer, length))
    {
        std::ofstream of("test_sheet.json");
        if(!of.bad())
        {
            smartjson::Writer writer;
            writer.write(parser.getRoot(), of);
            of.close();
        }
    }
    else
    {
        std::cerr << "Failed parse file " << fileName << ", code:" << parser.getErrorCode() << std::endl;
    }
    
    delete [] buffer;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";

    testString();
    testNode();
    testParser();
    testBinaryParser();
    
    std::cout << "test finished." << std::endl;
    return 0;
}
