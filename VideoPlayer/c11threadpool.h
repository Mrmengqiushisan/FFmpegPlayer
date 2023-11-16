#pragma once
#include<iostream>
#include<thread>
#include<mutex>
#include<functional>
#include<condition_variable>
#include<unordered_map>
#include <memory>
class ThreadPool{
public:
    static bool init(int threadsNum = 10, int tasksNum = 15);
    static bool addTask(std::function<void(std::shared_ptr<void>)> func,std::shared_ptr<void> par);
    static void releasePool();
    static bool checkThreadValid(size_t index);
    static bool chackThreadStatus();
    inline static bool m_status = true;
private:
    typedef struct Task{
        std::function<void(std::shared_ptr<void>)>func;//任务函数
        std::shared_ptr<void>ptr;//任务函数参数
    }TASK,*PTASK;
    typedef struct Threads{
        std::thread::id id;
        bool isTerminate;
        bool isWorking;
    }THREADS,*PTHREADS;
    static void threadEventLoop(std::shared_ptr<void> arg);
    inline static int m_maxThreads = 0;
    inline static int m_freeThreads = 0;
    inline static int m_maxTasks = 0;
    inline static int m_pushIndex = 0;
    inline static int m_readIndex = 0;
    inline static int m_size = 0;//这个size指的是任务队列的长度
    inline static int m_initFlag = -1;
    static std::unordered_map<std::thread::id, Task> thread_task;
    static std::vector<Threads> m_threadsQueue;
    static std::mutex m_mutex;
    static std::condition_variable m_cond;
    static std::vector<Task> m_tasksQueue;
};
