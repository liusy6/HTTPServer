#pragma once
#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

//定义时间节点结构
struct TimerNode {
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};

class Timer {
public:
    Timer() { heap_.reserve(64);} 
    ~Timer() { clear(); }   
    void adjust(int id, int newExpires);
    void add(int id, int timeOut, const TimeoutCallBack& cb);
    void doWork(int id);
    void clear();
    void tick();
    void pop();
    int GetNextTick();

private:
    void del(size_t i);
    void siftup(size_t i);
    bool siftdown(size_t index, size_t n);
    void SwapNode(size_t i, size_t j);
    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_;
};
