//
//  main.cpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#include <iostream>
#include "string.hpp"
#include "array.hpp"
#include "dict.hpp"
#include "node.hpp"
#include "allocator.hpp"

#include <string>
#include <cassert>

#define TEST_EQUAL(EXP) testEqual(EXP, #EXP, __LINE__)
void testEqual(bool ret, const char *exp, int line)
{
    if(!ret)
    {
        std::cout << "TestFailed: (" << exp << "), line: " << line << std::endl;
        abort();
    }
}

void testString()
{
    std::cout << "test string..." << std::endl;
    
    mjson::IAllocator *allocator = new mjson::RawAllocator();
    allocator->retain();
    
    std::string str("Hello World");
    mjson::String *p = allocator->createString(str.c_str(), str.size());
    p->retain();
    
    TEST_EQUAL(p->getRefCount() == 1);
    TEST_EQUAL(p->size() == str.size());
    TEST_EQUAL(str == p->data());
    TEST_EQUAL(p->compare(str.c_str()) == 0);
    
    mjson::String *p2 = (mjson::String*)p->clone();
    p2->retain();
    TEST_EQUAL(p->compare(p2) == 0);
    p2->release();
    
    mjson::String *p3 = (mjson::String*)p->deepClone();
    p3->retain();
    TEST_EQUAL(p->compare(p3) == 0);
    p3->release();
    
    p->release();
    
    allocator->release();
}

void testArray()
{
    std::cout << "test array..." << std::endl;
    
    
    mjson::RawAllocator allocator;
    allocator.retain();
    
    mjson::Array *p = allocator.createArray();
    p->retain();
    
    TEST_EQUAL(p->size() == 0);
    TEST_EQUAL(p->capacity() == 0);
    TEST_EQUAL(p->begin() == p->end());
    TEST_EQUAL(p->begin() == nullptr);
    
    p->append(true);
    p->append(1234567890);
    p->append(3.14);
    p->append("Hello World");
    TEST_EQUAL(p->size() == 4);
    TEST_EQUAL((*p)[0] == true);
    TEST_EQUAL((*p)[1] == 1234567890);
    TEST_EQUAL((*p)[2] == 3.14);
    TEST_EQUAL(strcmp((*p)[3].asCString(), "Hello World") == 0);
    
    TEST_EQUAL(p->capacity() == 4);
    p->append(mjson::Node());
    TEST_EQUAL(p->capacity() == 8);
    (*p)[4] = 1314;
    TEST_EQUAL((*p)[4] == 1314);
    
    mjson::Array *copy = (mjson::Array*)p->clone();
    copy->retain();
    
    TEST_EQUAL(copy->size() == p->size());
    TEST_EQUAL(copy->capacity() == p->size());
    for(int i = 0; i < p->size(); ++i)
    {
        TEST_EQUAL((*copy)[i] == (*p)[i]);
    }
    
    copy->release();
    p->release();
}

void testDict()
{
    
}

void testNode()
{
    
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    testString();
    testArray();
    testDict();
    testNode();
    return 0;
}
