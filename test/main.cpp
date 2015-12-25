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
#include "allocator.hpp"

#include <string>
#include <cassert>

#define TEST_EQUAL(EXP) testEqual(EXP, #EXP, __LINE__)
void testEqual(bool ret, const char *exp, int line)
{
    if(!ret)
    {
        std::cout << "TestFailed: (" << exp << "), line: " << line << std::endl;
    }
}

void testString()
{
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
    
    p->release();
    
    allocator->release();
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    testString();
    return 0;
}
