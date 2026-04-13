#pragma once

#include <functional>
#include <string>
#include <vector>
#include <memory>

#include "noncopyable.h"
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    // 如果工作在多线程中，baseLoop_(mainLoop)会默认以轮询的方式分配Channel给subLoop
    EventLoop *getNextLoop();

    std::vector<EventLoop *> getAllLoops(); // 获取所有的EventLoop

    bool started() const { return started_; } // 是否已经启动
    const std::string name() const { return name_; } // 获取名字

private:
    // 用户使用muduo创建的mainLoop，对应mainReactor
    // 如果numThreads_设置为0，则回退到单Reactor模型
    EventLoop *baseLoop_; 

    //线程池名称，通常由用户指定，线程池中EventLoopThread名称依赖于线程池名称。
    std::string name_;

    bool started_;//是否已经启动标志

    //线程池中的线程数量，对应subReactor的数量，专门负责与已建立连接的客户端之间进行业务处理
    int numThreads_;

    int next_; // 新连接到来，所选择EventLoop的索引

    std::vector<std::unique_ptr<EventLoopThread>> threads_;//IO线程的列表
    
    // 对应subReactors，线程池中EventLoop的列表，指向的是EVentLoopThread线程函数创建的EventLoop对象。
    std::vector<EventLoop *> loops_; 
};