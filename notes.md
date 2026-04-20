# 定性
TCP server

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
Callback.h
TcpServer.h TcpServer.cc 
Buffer.h Buffer.cc
TcpConnection.h TcpConnection.cc

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

# 整体框架快速理解
https://github.com/qingshan-z/cpp-study_shilei/blob/main/%E3%80%90%E9%AB%98%E7%BA%A7%E3%80%91%E6%89%8B%E5%86%99C%2B%2B%20Muduo%E7%BD%91%E7%BB%9C%E5%BA%93%E9%A1%B9%E7%9B%AE-%E6%8E%8C%E6%8F%A1%E9%AB%98%E6%80%A7%E8%83%BD%E7%BD%91%E7%BB%9C%E5%BA%93%E5%AE%9E%E7%8E%B0%E5%8E%9F%E7%90%86/%E7%AC%94%E8%AE%B0/36%20TcpServer%E4%BB%A3%E7%A0%81%E8%AE%B2%E8%A7%A3%E4%BA%8C%EF%BC%8843%E5%88%86%E9%92%9F%EF%BC%89.md