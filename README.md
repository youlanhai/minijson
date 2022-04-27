# smartjson
易于使用的Json库。使用智能指针来管理容器内存，降低拷贝代价。

home: https://github.com/youlanhai/smartjson

### 特性
- 使用智能指针管理容器，可避免节点之间赋值引起内存拷贝或所有权转移的问题;
- 支持字符串、数字、布尔类型当字典的key;
- 支持将json存贮为二进制文件格式，可加速配置文件解析和压缩重复key占用的空间;
- 支持解析带注释的json文件;
- 支持数组和字典尾部元素增加','，可避免json文件在版本控制中频繁冲突;
- 支持C++11语法。

# 如何编译
+ 安装最新版本的CMake，并将cmake.exe所在目录添加到环境变量`PATH`中。
```
mkdir build
cd build
cmake -G "your_generator" ../
```
将"your_generator"替换为你当前操作系统支持的编译器。  
+ 比如针对Visual Studio 2017可以这样写: `cmake -G "Visual Studio 15 Win64" ../ `;
+ 针对Xcode，可以这样写: `cmake -G Xcode ../ `;
+ 更多生成器使用`cmake -h`来查看。

# 用法
### 包含头文件
`#include "smartjson.hpp"`

### 使用命名空间smartjson
`using smartjson;`

### 解析Json
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

### 格式化Json为字符串
```c++
Writer writer;
writer.write(root, std::cout);
// or
std::cout << writer.toString(root) << std::endl;
// or
writer.writeToFile(root, "output.json");
```

# 值类型转换
## boolean
```c++
Node node(true);
// or
node = true;

bool b = node.asBool();
// or
b = node.as<bool>();
```
**注意:** 对于非bool类型的节点，asBool始终返回false。

## number
```c++
Node node(123456);
// or
node = 123456.789;

int a = node.asInt(); // a is 123456
// or
a = node.as<int>();

double b = node.asFloat(); // b is 123456.789
// or
b = node.as<double>();
```
+ 如果node是int类型，调用asFloat()会自动把int转换为float;
+ 如果node是float类型，调用asInt()会自动把float转换为int。
+ 注意: rawInt()和rawFloat()函数，之间返回内部数据格式，不会自动转换类型。

## string
```c++
Node node("hello！smartjson.");
// or
node = "hello！smartjson.";

const char *s = node.asCString();
// or
std::string s2 = node.as<std::string>();
```

+ 解析字符串时支持`\u1234`和`\x12`格式

## dict
### 基本用法
```c++
// 创建一个字典
Node node(T_DICT);
// or
node.setDict();

// 设置成员
node.setMember("name", "LanhaiYou");
node.setMember("age", 25);

// 获取成员
auto name = node.get<std::string>("name");
int age = node.get<int>("age");
// or
name = node["name"].as<std::string>();
age = node["age"].as<int>();
```

### 遍历字典
```c++
if(node.isDict())
{
    // visit each element
    for(DictIerator it = node.memberBegin(); it != node.memberEnd(); ++it)
    {
        std::cout << it->first << " " << it->second << std::endl;
    }
    // or
    for (auto &pair : node.refDict())
    {
        std::cout << pair.first << " " << pair.second << std::endl;
    }
}

```

### 容器内存共享
容器是使用引用计数机制来管理的，Node可以当做是一个智能指针。Node之间赋值，不会造成容器拷贝。
```c++
// node2和node指向的是同一个字典容器
Node node2 = node;
assert(node.rawDict() == node2.rawDict()); // assert never happens.
```

需要注意的是，不要把Node设置给自己或子节点，否则会发生循环引用，造成内存泄漏。如：
```c++
Node node(T_DICT);
node["self"] = node; // 错误用法！会造成循环引用
```

### 扩展自定义数据类型
假设有如下数据类型Pos，可以重写toNode和fromNode方法，让Node的模板方法能够识别自定义数据类型:
```c++
struct Pos
{
    float x, y;
};

void toNode(Node &node, const Pos &pos)
{
    node.setMember("x", pos.x);
    node.setMember("y", pos.y);
}

void fromNode(Pos &pos, const Node &node)
{
    pos.x = node.get<float>("x");
    pos.y = node.get<float>("y");
}
```

```c++
Pos pos = {1, 2};

Node node(pos);
// or
node = Node(pos);

pos = node.as<Pos>();

Node node2(T_DICT);
node2.setMember("pos", pos);

pos = node2.get<Pos>("pos");
// or
pos = node2["pos"].as<Pos>();
```

### 递归获取子节点
出来返回迭代器的方法外，其余方法都支持key**以路径的形式**，递归操作子节点。

```c++
Node node(T_DICT);

node.setMember("pos/x", 1.2f);
node.setMember("pos/y", 0.1f);

float x = node.get<float>("pos/x");
float y = node.get<float>("pos/y");
// or
x = node["pos"]["x"];
y = node["pos"]["y"];
```

## array

### 创建数组
```c++
Node node(T_ARRAY);
// or
node.setArray();

// 预留空间
node.reserve(100);

node.pushBack(1);
node.pushBack(true);
node.pushBack("hello");
node[0] = 2;
node[1] = false;

node.pushBack(3.14);
node.popBack();

Node node2 = node;
assert(node2.rawArray() == node.rawArray()); // assert never happens
```

### 遍历数组
```c++
if(root.isArray())
{
    for(ArrayIterator it = root.begin(); it != root.end(); ++it)
    {
        std::cout << *it << std::endl;
    }
    // or
    for (const Node & v : root)
    {
        std::cout << v << std::endl;
    }
    // or
    for(size_t i = 0; i < root.size(); ++i)
    {
        std::cout << root[i] << std::endl;
    }
}
```
