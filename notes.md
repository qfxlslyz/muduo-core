# 编写顺序：
noncopyable.h
Logger.h Logger.cc
Timestamp.h Timestamp.cc
InetAddress.h InetAddress.cc
Channel.h Channel.cc
Poller.h Poller.cc
DefaultPoller.cc
EPollPoller.h EPollPoller.cc
CurrentThread.h CurrentThread.cc
EventLoop.h EventLoop.cc
Thread.h Thread.cc
EventLoopThread.h EventLoopThread.cc
EventLoopThreadPool.h EventLoopThreadPool.cc
Socket.h Socket.cc
Acceptor.h Acceptor.cc

# 启动服务端
./testserver

# 测试 100 个并发连接 每次发送 1400 字节 持续 10 秒
./echo_bench -h 127.0.0.1 -p 8080 -c 100 -m 1400 -t 10

# 输出指标：
Total requests
总共完成了多少次“发一条 + 收一条”
QPS / RPS
每秒完成多少次 echo 往返
Recv throughput
接收方向吞吐量，单位 MiB/s
Average RTT
单次 echo 往返平均耗时，单位微秒