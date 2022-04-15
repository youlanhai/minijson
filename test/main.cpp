//
//  main.cpp
//  smartjson
//
//  Created by youlanhai on 16/1/1.
//  Copyright © 2016年 youlanhai. All rights reserved.
//

#include <iostream>

#include "smartjson.hpp"

const char *help = R"(conver json to binary data.
usage: smartjson input [output]
)";

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cout << help << std::endl;
        return 0;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile;
    if(argc > 2)
    {
        outputFile = argv[2];
    }
    
    if(outputFile.empty())
    {
        outputFile = inputFile + ".out";
    }
    
    smartjson::Parser parser;
    if (!parser.parseFromFile(inputFile))
    {
        std::cout << "Parse json Failed: code :" << parser.getErrorCode() << std::endl
            << "line: " << parser.getLine() << std::endl
            << "column: " << parser.getColumn() << std::endl;
        return -1;
    }
    
    smartjson::BinaryWriter writer;
    if (!writer.writeToFile(parser.getRoot(), outputFile))
    {
        std::cout << "Write binary data file Failed: " << parser.getErrorCode() << std::endl;
        return -1;
    }
    return 0;
}
