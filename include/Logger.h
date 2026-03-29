#pragma once

#include <string>

#include "noncopyable.h"


/**
 * 笔记：
 * snprintf(buf, 1024, logmsgFormat, ##VA_ARGS);
 * 含义是：
 * 如果写了额外参数
 * 例如 LOG_INFO("%s %d", name, age)
 * 那么 ... 会正常展开为可变参数列表 name, age
 * 最终执行 snprintf(buf, 1024, "%s %d", name, age);
 * 
 * 如果没写额外参数
 * 例如 LOG_INFO("start ok")
 * 那么 ## 会把前面的那个逗号去掉，避免变成非法的
 * 最终执行 snprintf(buf, 1024, "start ok");
 * 
 * do { ... } while (0) 主要解决 3 个问题：
 * 把“多条语句宏”包装成“一条语句”
 * 避免 if/else 绑定错误
 * 允许宏内部定义局部变量且作用域清晰
 */

// 使用方法：LOG_INFO("%s %d", arg1, arg2)  LOG_INFO("status ok")
#define LOG_INFO(logmsgFormat, ...)                       \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(INFO);                         \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)

#define LOG_ERROR(logmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(ERROR);                        \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)

#define LOG_FATAL(logmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(FATAL);                        \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
        exit(-1);                                         \
    } while (0)

#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(DEBUG);                        \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif

// 定义日志的级别 INFO ERROR FATAL DEBUG
enum LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
    FATAL, // core dump信息
    DEBUG, // 调试信息
};

// 输出一个日志类
class Logger : noncopyable
{
public:
    // 获取日志唯一的实例对象 单例模式
    static Logger& instance();
    // 设置日志级别
    void setLogLevel(int level);
    // 写日志
    void log(std::string msg);

private:
    int logLevel_;  // 编码习惯：成员变量命名时末尾加下划线 _
};