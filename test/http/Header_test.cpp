#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <vector>
#include <map>
#include <doctest/doctest.h>
#include "http/Header.h"

using namespace esynet::http;
using namespace std;

TEST_CASE("Header_Test"){
    Header header;

    string str = "test";
    string_view sv = str;

    int num = 123;
    double num2 = 123.456;
    float num3 = 3.14;
    unsigned int num4 = 12;
    short num5 = 1;
    unsigned short num6 = 1;
    long long num7 = 123456789;
    unsigned long long num8 = 123456789;
    bool flag = true;

    vector<string> v1 = {"test1", "test2", "test3"};
    vector<int> v2 = {1, 2, 3};

    map<string, string> m1 = {{"key1", "value1"}, {"key2", "value2"}};
    map<string, int> m2 = {{"key1", 1}, {"key2", 2}};
    map<int, string> m3 = {{1, "value1"}, {2, "value2"}};
    map<int, int> m4 = {{1, 1}, {2, 2}};
    unordered_map<string, string> m5 = {{"key1", "value1"}, {"key2", "value2"}};
    multimap<string, string> m6 = {{"key1", "value1"}, {"key2", "value2"}};
    unordered_multimap<string, string> m7 = {{"key1", "value1"}, {"key2", "value2"}};

    set<string> s1 = {"st1", "st2", "st3"};
    set<int> s2 = {1, 2, 3};
    unordered_set<string> s3 = {"st1", "st2", "st3"};
    unordered_multiset<string> s4 = {"st1", "st2", "st3"};
    multiset<string> s5 = {"st1", "st2", "st3"};

    header.set("string", str);
    CHECK(header.get("string").value().first->second == "test");
    header.set("string_view", sv);
    CHECK(header.get("string_view").value().first->second == "test");

    header.set("int", num);
    CHECK(header.get("int").value().first->second == "123");
    header.set("double", num2);
    CHECK(header.get("double").value().first->second == "123.456000");
    header.set("float", num3);
    CHECK(header.get("float").value().first->second == "3.140000");
    header.set("unsigned int", num4);
    CHECK(header.get("unsigned int").value().first->second == "12");
    header.set("short", num5);
    CHECK(header.get("short").value().first->second == "1");
    header.set("unsigned short", num6);
    CHECK(header.get("unsigned short").value().first->second == "1");
    header.set("long long", num7);
    CHECK(header.get("long long").value().first->second == "123456789");
    header.set("unsigned long long", num8);
    CHECK(header.get("unsigned long long").value().first->second == "123456789");
    header.set("bool", flag);
    CHECK(header.get("bool").value().first->second == "true");

    header.set("vector<string>", v1);
    CHECK(header.get("vector<string>").value().first->second == "[test1,test2,test3]");
    header.set("vector<int>", v2);
    CHECK(header.get("vector<int>").value().first->second == "[1,2,3]");

    header.set("map<string, string>", m1);
    CHECK(header.get("map<string, string>").value().first->second == "{key1:value1,key2:value2}");
    header.set("map<string, int>", m2);
    CHECK(header.get("map<string, int>").value().first->second == "{key1:1,key2:2}");
    header.set("map<int, string>", m3);
    CHECK(header.get("map<int, string>").value().first->second == "{1:value1,2:value2}");
    header.set("map<int, int>", m4);
    CHECK(header.get("map<int, int>").value().first->second == "{1:1,2:2}");
    header.set("unordered_map<string, string>", m5);
    CHECK(header.get("unordered_map<string, string>").value().first->second == "{key2:value2,key1:value1}");
    header.set("multimap<string, string>", m6);
    CHECK(header.get("multimap<string, string>").value().first->second == "{key1:value1,key2:value2}");
    header.set("unordered_multimap<string, string>", m7);
    CHECK(header.get("unordered_multimap<string, string>").value().first->second == "{key2:value2,key1:value1}");

    header.set("set<string>", s1);
    CHECK(header.get("set<string>").value().first->second == "{st1,st2,st3}");
    header.set("set<int>", s2);
    CHECK(header.get("set<int>").value().first->second == "{1,2,3}");
    header.set("unordered_set<string>", s3);
    CHECK(header.get("unordered_set<string>").value().first->second == "{st3,st2,st1}");
    header.set("unordered_multi_set<string>", s4);
    CHECK(header.get("unordered_multi_set<string>").value().first->second == "{st3,st2,st1}");
    header.set("multi_set<string>", s5);
    CHECK(header.get("multi_set<string>").value().first->second == "{st1,st2,st3}");
}