#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

struct Config {
    std::string host = "127.0.0.1";
    int port = 8080;
    int concurrency = 10;
    int msg_size = 64;
    int duration_sec = 10;
};

struct Stats {
    std::atomic<long long> total_requests{0};
    std::atomic<long long> total_bytes_sent{0};
    std::atomic<long long> total_bytes_recv{0};
    std::atomic<long long> total_latency_ns{0};
    std::atomic<long long> failed_requests{0};
};

static void print_usage(const char* prog) {
    std::cout
        << "Usage: " << prog << " [-h host] [-p port] [-c concurrency] [-m msg_size] [-t duration]\n"
        << "  -h host          Server IP (default: 127.0.0.1)\n"
        << "  -p port          Server port (default: 8080)\n"
        << "  -c concurrency   Number of concurrent connections (default: 10)\n"
        << "  -m msg_size      Message size in bytes (default: 64)\n"
        << "  -t duration      Test duration in seconds (default: 10)\n";
}

static bool parse_args(int argc, char* argv[], Config& cfg) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" && i + 1 < argc) {
            cfg.host = argv[++i];
        } else if (arg == "-p" && i + 1 < argc) {
            cfg.port = std::stoi(argv[++i]);
        } else if (arg == "-c" && i + 1 < argc) {
            cfg.concurrency = std::stoi(argv[++i]);
        } else if (arg == "-m" && i + 1 < argc) {
            cfg.msg_size = std::stoi(argv[++i]);
        } else if (arg == "-t" && i + 1 < argc) {
            cfg.duration_sec = std::stoi(argv[++i]);
        } else {
            print_usage(argv[0]);
            return false;
        }
    }

    if (cfg.port <= 0 || cfg.port > 65535 ||
        cfg.concurrency <= 0 ||
        cfg.msg_size <= 0 ||
        cfg.duration_sec <= 0) {
        std::cerr << "Invalid arguments.\n";
        return false;
    }

    return true;
}

static int connect_server(const std::string& host, int port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        std::cerr << "inet_pton failed for host: " << host << "\n";
        ::close(fd);
        return -1;
    }

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("connect");
        ::close(fd);
        return -1;
    }

    return fd;
}

static bool send_all(int fd, const char* buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = ::send(fd, buf + sent, len - sent, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (n == 0) {
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}

static bool recv_all(int fd, char* buf, size_t len) {
    size_t recvd = 0;
    while (recvd < len) {
        ssize_t n = ::recv(fd, buf + recvd, len - recvd, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (n == 0) {
            return false;
        }
        recvd += static_cast<size_t>(n);
    }
    return true;
}

static void worker(const Config& cfg,
                   Stats& stats,
                   std::atomic<bool>& stop_flag,
                   int worker_id) {
    (void)worker_id;

    int fd = connect_server(cfg.host, cfg.port);
    if (fd < 0) {
        stats.failed_requests.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    std::vector<char> send_buf(static_cast<size_t>(cfg.msg_size), 'A');
    std::vector<char> recv_buf(static_cast<size_t>(cfg.msg_size), 0);

    while (!stop_flag.load(std::memory_order_relaxed)) {
        auto start = std::chrono::steady_clock::now();

        if (!send_all(fd, send_buf.data(), send_buf.size())) {
            stats.failed_requests.fetch_add(1, std::memory_order_relaxed);
            break;
        }

        if (!recv_all(fd, recv_buf.data(), recv_buf.size())) {
            stats.failed_requests.fetch_add(1, std::memory_order_relaxed);
            break;
        }

        auto end = std::chrono::steady_clock::now();
        long long latency_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        stats.total_requests.fetch_add(1, std::memory_order_relaxed);
        stats.total_bytes_sent.fetch_add(static_cast<long long>(send_buf.size()), std::memory_order_relaxed);
        stats.total_bytes_recv.fetch_add(static_cast<long long>(recv_buf.size()), std::memory_order_relaxed);
        stats.total_latency_ns.fetch_add(latency_ns, std::memory_order_relaxed);
    }

    ::close(fd);
}

int main(int argc, char* argv[]) {
    Config cfg;
    if (!parse_args(argc, argv, cfg)) {
        return 1;
    }

    std::cout << "Echo benchmark starting...\n";
    std::cout << "Server      : " << cfg.host << ":" << cfg.port << "\n";
    std::cout << "Concurrency : " << cfg.concurrency << "\n";
    std::cout << "Message size: " << cfg.msg_size << " bytes\n";
    std::cout << "Duration    : " << cfg.duration_sec << " sec\n";

    Stats stats;
    std::atomic<bool> stop_flag{false};
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(cfg.concurrency));

    auto bench_start = std::chrono::steady_clock::now();

    for (int i = 0; i < cfg.concurrency; ++i) {
        threads.emplace_back(worker, std::cref(cfg), std::ref(stats), std::ref(stop_flag), i);
    }

    std::this_thread::sleep_for(std::chrono::seconds(cfg.duration_sec));
    stop_flag.store(true, std::memory_order_relaxed);

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    auto bench_end = std::chrono::steady_clock::now();
    double elapsed_sec =
        std::chrono::duration_cast<std::chrono::duration<double>>(bench_end - bench_start).count();

    long long total_requests = stats.total_requests.load(std::memory_order_relaxed);
    long long total_bytes_sent = stats.total_bytes_sent.load(std::memory_order_relaxed);
    long long total_bytes_recv = stats.total_bytes_recv.load(std::memory_order_relaxed);
    long long total_latency_ns = stats.total_latency_ns.load(std::memory_order_relaxed);
    long long failed_requests = stats.failed_requests.load(std::memory_order_relaxed);

    double qps = elapsed_sec > 0 ? static_cast<double>(total_requests) / elapsed_sec : 0.0;
    double throughput_mbps =
        elapsed_sec > 0 ? static_cast<double>(total_bytes_recv) / elapsed_sec / 1024.0 / 1024.0 : 0.0;
    double avg_rtt_us =
        total_requests > 0 ? static_cast<double>(total_latency_ns) / total_requests / 1000.0 : 0.0;

    std::cout << "\n===== Result =====\n";
    std::cout << "Elapsed time       : " << std::fixed << std::setprecision(3) << elapsed_sec << " s\n";
    std::cout << "Total requests     : " << total_requests << "\n";
    std::cout << "Failed requests    : " << failed_requests << "\n";
    std::cout << "QPS / RPS          : " << std::fixed << std::setprecision(2) << qps << "\n";
    std::cout << "Bytes sent         : " << total_bytes_sent << "\n";
    std::cout << "Bytes received     : " << total_bytes_recv << "\n";
    std::cout << "Recv throughput    : " << std::fixed << std::setprecision(2) << throughput_mbps << " MiB/s\n";
    std::cout << "Average RTT        : " << std::fixed << std::setprecision(2) << avg_rtt_us << " us\n";

    return 0;
}