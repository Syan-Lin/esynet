#pragma once

#include <memory>
#include <mutex>

class Singletonable {
public:
    Singletonable(const Singletonable&)  = delete;
    void operator=(const Singletonable&) = delete;
    Singletonable(Singletonable&&)       = delete;
    void operator=(Singletonable&&)      = delete;

protected:
    Singletonable()  = default;
    ~Singletonable() = default;
};

template <typename T>
class Singleton : Singletonable {
public:
    static T& instance() {
        std::call_once(init_flag_, [&](){
            instance_ = std::make_unique<T>();
        });
        return *instance_;
    }
    template<typename... Args>
    static T& instance(Args&&... args) {
        std::call_once(init_flag_, [&](){
            instance_ = std::make_unique<T>(std::forward<Args&&>(args)...);
        });
        return *instance_;
    }
    static void destroy() {
        std::call_once(dest_flag_, [&](){
            instance_.reset();
        });
    }

private:
    Singleton()  = default;
    ~Singleton() = default;

private:
    static std::unique_ptr<T> instance_;
    static std::once_flag init_flag_;
    static std::once_flag dest_flag_;
};

template<typename T>
std::unique_ptr<T> Singleton<T>::instance_ = nullptr;

template<typename T>
std::once_flag Singleton<T>::init_flag_;

template<typename T>
std::once_flag Singleton<T>::dest_flag_;
