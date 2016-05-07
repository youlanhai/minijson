# smartjson
a simple json library like rapidjson, but easy and safe to use.
smartjson uses smart pointer to manage memory. 

home: https://github.com/youlanhai/smartjson

# usage
##include the header
    ```c++
    #include "smartjson/src/json.hpp"
    ```
##parse from text
    ```c++
    mjson::Parser parser;
    int ret = parser.parse(text, strlen(text));
    if(ret != mjson::RC_OK)
    {
        return;
    }
    mjson::Node root = parser.getRoot();
    ```
##write to stream
    ```c++
    mjson::Writer writer;
    writer.write(root, std::cout);
    ```
##boolean
    ```
    mjson::Node node = true;
    bool b = node.asBool(); // b is true.
    ```
**notice:** only 'true' is true, anything else is false. 
##number
    ```
    mjson::Node node = 123456.789;
    int a = node.asInt(); // a is 123456
    double b = node.asFloat(); // b is 123456.789
    ```
convert between integer and float is safe.
##string
    ```
    mjson::Node node = "hello！smart json.";
    const char *s = node.asCString();
    std::string s2 = node.asStdString();
    std::string s3;
    node.asStdString(s3);
    ```
##dict
    ```
    // visit dict.
    if(root.isDict())
    {
        mjson::Dict &dict = root.refDict();
        // visit each element
        for(mjson::Dict::iterator it = dict.begin();
            it != dict.end(); ++it)
        {
            std::cout << it->key.asCString()
                << it->value.asCString() << std::endl;
        }
    }
    // create a dict.
    mjson::Node node;
    node.setDict();
    node["name"] = "LanhaiYou";
    node["age"] = 25;
    // assign a dict node to other node, will not cause memory allocation.
    // both two node reference to the same dict.
    mjson::Node node2 = node;
    assert(node.rawDict() == node2.rawDict()); // assert will not happen.
    ```
##array
    ```
    // visit array.
    if(root.isArray())
    {
        mjson::Array &array = root.refArray();
        // visit each element
        for(mjson::Array::iterator it = array.begin();
            it != array.end(); ++it)
        {
            std::cout << it->asCString() << std::endl;
        }
        // or
        for(size_t i = 0; i < array.size(); ++i)
        {
            std::cout << array[i].asInt() << std::endl;
        }
    }
    // create array.
    mjson::Node node;
    mjson::Aray *p = node.setArray();
    p->resize(4);
    p->append(1);
    p->append(true);
    p->append("hello");
    node[0u] = 2; // use '0u' instead of '0'.
    node[3] = false;
    ```

