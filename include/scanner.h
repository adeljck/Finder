#pragma once
#include <string>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <vector>
#include "config.h"

class DirQueue {
public:
    void push(std::wstring p);
    bool pop(std::wstring &out);
    void task_done();
private:
    std::queue<std::wstring> q;
    std::mutex m;
    std::condition_variable cv;
    size_t pending = 0;
    bool stop = false;
};

void run_workers(DirQueue &dq, const Config &cfg);


