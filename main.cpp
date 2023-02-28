#include "utils/Timestamp.h"
#include "utils/Singleton.h"
#include <dbg.h>
#include "logger/Logger.h"
#include "utils/Buffer.h"
#include "utils/FileUtil.h"
#include "logger/AsyncLogger.h"
#include <fmt/format.h>
#include <fmt/color.h>

#include <iomanip>
#include <iostream>
#include <chrono>
#include <thread>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <filesystem>

using namespace std;
using namespace fmt;

atomic<int> t1 = 0;
atomic<int> t2 = 0;
atomic<int> t3 = 0;
atomic<int> t4 = 0;
atomic<int> t5 = 0;
int times = 100000;
int sleepTime = 0;

void thread1() {
    for(int i = 0; i < times; i++) {
        LOG_DEBUG("LOG_DEBUG: {}", t1++);
        // this_thread::sleep_for(chrono::milliseconds(sleepTime));
    }
}

void thread2() {
    for(int i = 0; i < times; i++) {
        LOG_INFO("LOG_INFO: {}", t2++);
        // this_thread::sleep_for(chrono::milliseconds(sleepTime));
    }
}

void thread3() {
    for(int i = 0; i < times; i++) {
        LOG_ERROR("LOG_ERROR: {}", t3++);
        // this_thread::sleep_for(chrono::milliseconds(sleepTime));
    }
}

void thread4() {
    for(int i = 0; i < times; i++) {
        LOG_WARN("LOG_WARN: {}", t4++);
        // this_thread::sleep_for(chrono::milliseconds(sleepTime));
    }
}

void thread5() {
    for(int i = 0; i < times; i++) {
        LOG_FATAL("LOG_FATAL: {}", t5++);
        // this_thread::sleep_for(chrono::milliseconds(sleepTime));
    }
}

class Test {
public:
    Test() {
        cout << "Test()" << endl;
    }
    Test(const Test& t) {
        cout << "Test(const Test& t)" << endl;
    }
    Test(Test&& t) {
        cout << "Test(Test&& t)" << endl;
    }
    ~Test() {
        cout << "~Test()" << data_ << endl;
    }
    void print() {
        cout << "print()" << endl;
    }
    void call() {
        dbg(this);
        auto lamb = [this] {
            dbg(this);
            this->print();
        };
        lamb();
    }
    int data_ = 1;
};

#include <filesystem>
#include <fstream>
#include "utils/PerformanceAnalyser.h"
#include "logger/SyncLogger.h"
#include "net/Reactor.h"
#include "net/Event.h"
#include <sys/timerfd.h>

std::string gStrForTest;

// void timeOut(Reactor& loop) {
//     gStrForTest += "t";
//     loop.stop();
// }

int main() {
    // int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    // struct itimerspec howlong;
    // bzero(&howlong, sizeof howlong);
    // howlong.it_value.tv_sec = 2;
    // timerfd_settime(timer_fd, 0, &howlong, NULL);

    Reactor loop(false);
    Event event(loop, 1);
    // event.setReadCallback(std::bind(timeOut, std::ref(loop)));
    // event.enableReading(); /* 会委托 loop 更新 poll */

    // loop.loop();
    // close(timer_fd);

    return 0;
}

/*
template<typename Function>
void countTime(string name, Function&& func, int loop = 1, bool useMicroSeconds = false) {
    using namespace chrono;
    system_clock::time_point time_point_now = system_clock::now();
    for(int i = 0; i < loop; i++) {
        func();
    }
    system_clock::time_point time_point_after = system_clock::now();
    system_clock::duration duration = time_point_after - time_point_now;
    if(useMicroSeconds) {
        cout << name << " cost " << duration_cast<microseconds>(duration).count() << " μs" << endl;
    } else {
        cout << name << " cost " << duration_cast<milliseconds>(duration).count() << " ms" << endl;
    }
}

int main() {
    stringstream sstream;

    // 整型可以是 int, long long int 等等
    // 1. integer -> string
    sstream << -123;
    cout << sstream.str() << endl;
    sstream.str("");
    sstream.clear();

    // i 的类型可以替换为 long long int 等等
    // 2. string -> integer
    sstream << "-123";
    int i;
    sstream >> i;
    cout << i << endl;
    sstream.str("");
    sstream.clear();

    // 3. double -> string
    sstream << setprecision(11);
    sstream << -92.123456789;
    cout << sstream.str() << endl;
    sstream.str("");
    sstream.clear();

    // 4. string -> double
    sstream << "-92.123456789";
    double d;
    sstream >> d;
    cout << setprecision(11) << d << endl;
    cout << quoted("abcd") << 1234567889 << endl;
    sstream.str("");
    sstream.clear();

    int loop = 1;
    stringstream ss("9223372036854775807");
    long long int x;

    cout << "loop " << loop << " times" << endl;

    countTime("stringstream", [&](){
        ss >> x;
    }, loop, true);

    countTime("sscanf", [&](){
        sscanf(ss.str().c_str(), "%lld", &x);
    }, loop, true);

    countTime("itoa", [&](){
        x = atoll(ss.str().c_str());
    }, loop, true);

    countTime("stoi", [&](){
        x = stoll(ss.str());
    }, loop, true);

    // std::to_string(val)

    return 0;
}*/