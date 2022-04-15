# smartjson
a simple json library like rapidjson, but easy and safe to use.
smartjson uses smart pointer to manage memory. 

home: https://github.com/youlanhai/smartjson

# build
```
mkdir build
cd build
cmake -G"your_generator" ../
```
replace "your_generator" to one of cmake generators. use `cmake -h` to see all generators supported on your platform.

## cmake params for android
```
-G"Unix Makefiles"
-DCMAKE_TOOLCHAIN_FILE=../CMake/Toolchains/android.cmake
-DANDROID_NDK=${your_android_ndk}
-DANDROID_NATIVE_API_LEVEL=android-14
-DANDROID_ABI=armeabi
```
see `CMake/Toolchins/android.cmake` for more helpers.

## cmake params for ios
```
-DCMAKE_TOOLCHAIN_FILE=../CMake/Toolchains/iOS.cmake
```

# usage
## include the header
`#include "smartjson.hpp"`

## parse
```c++
Parser parser;
bool ret;
ret = parser.parseFromFile("input.json");
// or
ret = parser.parseFromString(text);
// or
ret = parser.parseFromData(data, dataLength);
// or
ret = parser.parse(std::cin);
if (!ret)
{
    std::cout << "parse failed: " << parser.getErrorCode() << std::endl;
    return;
}
Node root = parser.getRoot();
```

## write
```c++
Writer writer;
writer.write(root, std::cout);
// or
std::cout << writer.toString(root) << std::endl;
// or
writer.writeToFile(root, "output.json");
```

# value type
## boolean
```c++
Node node = true;
bool b = node.asBool(); // b is true.
// or
b = node.as<bool>();
```
**notice:** only 'true' is true, anything else is false. 

## number
```c++
Node node = 123456.789;
int a = node.asInt(); // a is 123456
// or
a = node.as<int>();

double b = node.asFloat(); // b is 123456.789
// or
b = node.as<double>();
```
convert between integer and float is safe.

## string
```c++
Node node = "hello！smart json.";
const char *s = node.asCString();
// or
std::string s2 = node.as<std::string>();
```

## dict
```c++
// visit dict.
if(root.isDict())
{
    // visit each element
    for(DictIerator it = root.memberBegin(); it != root.memberEnd(); ++it)
    {
        std::cout << it->first << " " << it->second << std::endl;
    }
    // or
    for (auto &pair : root.refDict())
    {
        std::cout << pair.first << " " << pair.second << std::endl;
    }
}

// create a dict.
Node node;
node.setDict();
// or
node = Node(T_DICT);

node.setMember("name", "LanhaiYou");
node.setMember("age", 25);

auto name = node.getMember<std::string>("name");
auto age = node.getMember<int>("age");
// or
name = node["name"].as<std::string>();
age = node["age"].as<int>();

// assign a dict node to other node, will not cause memory allocation.
// both two node reference to the same dict.
Node node2 = node;
assert(node.rawDict() == node2.rawDict()); // assert never happens.
```

```c++
Node node(T_DICT);

Node pos(T_DICT);
pos.setMember("x", 1.2f);
pos.setMember("y", 0.1f);
node.setMember("pos", pos);
// or
node.setMember("pos/x", 1.2f);
node.setMember("pos/y", 0.1f);

float x = node["pos"]["x"];
float y = node["pos"]["y"];
// or
x = node.getMember<float>("pos/x");
y = node.getMember<float>("pos/y");
// or
struct Pos
{
    float x, y;
};
void toNode(Node &node, const Pos &p)
{
    node.setMember("x", p.x);
    node.setMember("y", p.y);
}
void fromNode(Pos &p, const Node &node)
{
    p.x = node.getMember<float>("x");
    p.y = node.getMember<float>("y");
}

Pos p = pos.as<Pos>();
pos = p;

p = node.getMember<Pos>("pos");
node.setMember("pos", p);
```

## array
```c++
// visit array.
if(root.isArray())
{
    // visit each element
    for(ArrayIterator it = root.begin(); it != root.end(); ++it)
    {
        std::cout << *it << std::endl;
    }
    // or
    for(size_t i = 0; i < root.size(); ++i)
    {
        std::cout << root[i] << std::endl;
    }
    // or
    for (const Node & v : root)
    {
        std::cout << v << std::endl;
    }
}

// create array.
Node node;
node.setArray();
node.reserve(4);
node.pushBack(1);
node.pushBack(true);
node.pushBack("hello");
node[0] = 2;
node[1] = false;

Node node2 = node;
assert(node2.rawArray() == node.rawArray()); // assert never happens
```

