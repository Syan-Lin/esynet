#include "Timestamp.h"
#include "Singleton.h"
#include <dbg.h>
#include "Logger.h"
#include "Buffer.h"
#include "FileUtil.h"
#include "AsyncLogger.h"
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
int times = 1000000;
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
#include "PerformanceAnalyser.h"

/* Logger 最高写入速度可达 193 MB/s */
int main() {
    TimeStatistics ts;
    Singleton<TimeAnalyser>::instance().addStat(
        std::bind(
            &TimeStatistics::stat, &ts,
            std::placeholders::_1, std::placeholders::_2
        )
    );

    AsyncLogger al("log.log");
    Logger::setLogger(std::bind(&AsyncLogger::append, &al, std::placeholders::_1));
    al.start();

    Singleton<TimeAnalyser>::instance().start("main");

    for(int i = 0; i < times; i++) {
        LOG_INFO("LOG_INFO: {}", t1++);
    }

    // mutex mutex_;
    // for(int i = 0; i < 1000; i++) {
    //     unique_lock<mutex> lock(mutex_);
    // }

    // thread t1(thread1);
    // thread t2(thread2);
    // thread t3(thread3);
    // thread t4(thread4);
    // thread t5(thread5);
    // t1.join();
    // t2.join();
    // t3.join();
    // t4.join();
    // t5.join();

    Singleton<TimeAnalyser>::instance().stop("main");

    dbg(ts.toString(TimeAnalyser::SECONDS));

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