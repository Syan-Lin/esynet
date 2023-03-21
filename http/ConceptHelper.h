#pragma once

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <ranges>

template<typename T>
struct is_map : std::false_type {};
template<typename Key, typename Value>
struct is_map<std::map<Key, Value>> : std::true_type {};
template<typename Key, typename Value>
struct is_map<std::multimap<Key, Value>> : std::true_type {};
template<typename Key, typename Value>
struct is_map<std::unordered_map<Key, Value>> : std::true_type {};
template<typename Key, typename Value>
struct is_map<std::unordered_multimap<Key, Value>> : std::true_type {};

template<typename T>
struct is_set : std::false_type {};
template<typename Key>
struct is_set<std::set<Key>> : std::true_type {};
template<typename Key>
struct is_set<std::multiset<Key>> : std::true_type {};
template<typename Key>
struct is_set<std::unordered_set<Key>> : std::true_type {};
template<typename Key>
struct is_set<std::unordered_multiset<Key>> : std::true_type {};

template<typename T>
concept BooleanType = std::is_same_v<T, bool>;
template<typename T>
concept IntegerType = std::is_integral_v<T> && !BooleanType<T>;
template<typename T>
concept FloatingPointType = std::is_floating_point_v<T>;
template<typename T>
concept StringType = std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;
template<typename T>
concept SequentialContainerType = std::ranges::range<T> && !is_map<T>::value
                                && !is_set<T>::value && !StringType<T>;
template<typename T>
concept MapType = std::ranges::range<T> && is_map<T>::value;
template<typename T>
concept SetType = std::ranges::range<T> && is_set<T>::value;