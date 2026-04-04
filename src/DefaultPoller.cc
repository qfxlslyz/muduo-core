#include <stdlib.h>

#include "Poller.h"
#include "EPollPoller.h"

/**
 * 如果将基类Poller的成员函数newDefaultPoller的实现放在Poller.cc中
 * 那么基类实现文件Poller.cc需要包含派生类头文件EpollPoller.h，这与
 * 派生类实现依赖于基类实现，但是基类的实现不应该依赖派生类的实现的设计理念冲突
 * 因此选择单独写一个DefaultPoller.cc文件实现基类Poller的成员函数newDefaultPoller
 */

Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUDUO_USE_POLL"))
    {
        return nullptr; // 生成poll的实例
    }
    else
    {
        return new EPollPoller(loop); // 生成epoll的实例
    }
}