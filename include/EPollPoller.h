#pragma once

#include <vector>
#include <sys/epoll.h>

#include "Poller.h"
#include "Timestamp.h"

/**
 * epoll的使用:
 * 1. epoll_create
 * 2. epoll_ctl   add/mod/del
 * 3. epoll_wait
 **/

class Channel;

class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    /**
     * override 的关键作用总结
     * 编译时检查	确保函数确实覆盖了基类的虚函数
     * 防止笔误	   避免因参数类型、const修饰符等错误导致意外重载(override修饰的成员函数必须在基类中找到参数类型、const修饰符完全一致的虚函数)
     * 提高可读性	明确表明该函数的意图是覆盖基类函数
     * 维护性      修改基类函数签名时，派生类会编译报错提醒更新
     */

    // 重写基类Poller的抽象方法
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;

    // 填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    // 更新channel通道 其实就是调用epoll_ctl
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>; // C++中可以省略struct 直接写epoll_event即可

    int epollfd_;      // epoll_create创建返回的fd保存在epollfd_中
    EventList events_; // 用于存放epoll_wait返回的所有发生的事件的文件描述符事件集
};