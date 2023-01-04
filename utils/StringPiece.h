#pragma once

#include <string.h>
#include <string>

namespace esynet::utils {

/* 注意: 以下的 String 包装类并不控制字符串的生命周期 */

/* 使用 StringArg 就可以不用声明 const char* 和 std::string 两个版本的函数
 * StringArg 是 C 风格字符串 */
class StringArg {
public:
    StringArg(const char* str)        : str_(str) {}
    StringArg(const std::string& str) : str_(str.c_str()) {}
    const char* c_str() const { return str_; }

private:
    const char* str_;
};

/* 使用 StringPiece 就可以不用声明 const char* 和 std::string 两个版本的函数
 * StringPiece 是 C++ 风格字符串 */
class StringPiece {
private:
    const char* ptr_;
    size_t      length_;

public: 
    StringPiece() : ptr_(nullptr), length_(0) {}
    StringPiece(const char* str) : ptr_(str), length_(strlen(ptr_)) {}
    StringPiece(const unsigned char* str) : ptr_(reinterpret_cast<const char*>(str)),
                                            length_(strlen(ptr_)) {}
    StringPiece(const std::string& str) : ptr_(str.data()), length_(str.size()) {}
    StringPiece(const char* offset, int len) : ptr_(offset), length_(len) {}

    /* data() 返回的字符串指针有可能不包含终止符（'\0'）
     * 如果明确需要终止符，请使用 'asString().c_str()' */
    const char* data()  const { return ptr_; }
    int         size()  const { return length_; }
    bool        empty() const { return length_ == 0; }
    const char* begin() const { return ptr_; }
    const char* end()   const { return ptr_ + length_; }

    void clear() { ptr_ = nullptr; length_ = 0; }
    void set(const char* buffer, int len) { ptr_ = buffer; length_ = len; }
    void set(const char* str) { ptr_ = str; length_ = strlen(str); }
    void set(const void* buffer, int len) {
        ptr_ = reinterpret_cast<const char*>(buffer);
        length_ = len;
    }

    char operator[](size_t i) const { return ptr_[i]; }

    void removePrefix(int n) {
        ptr_ += n;
        length_ -= n;
    }

    void removeSuffix(int n) {
        length_ -= n;
    }

    bool operator==(const StringPiece& x) const {
        return ((length_ == x.length_) && (memcmp(ptr_, x.ptr_, length_) == 0));
    }
    bool operator!=(const StringPiece& x) const {
        return !(*this == x);
    }

    std::string asString() const {
        return std::string(data(), size());
    }

    void copyToString(std::string* target) const {
        target->assign(ptr_, length_);
    }

    // Does "this" start with "x"
    bool startsWith(const StringPiece& x) const {
        return ((length_ >= x.length_) && (memcmp(ptr_, x.ptr_, x.length_) == 0));
    }
};

#ifdef HAVE_TYPE_TRAITS
/* This makes vector<StringPiece> really fast for some STL implementations */
template<> struct __type_traits<muduo::StringPiece> {
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
#endif

} /* namespace esynet::utils */