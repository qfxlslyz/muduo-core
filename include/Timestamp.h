#pragma once

#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    /**
     * 笔记
     * explicit 修饰有参构造函数用来防止意外的自动类型转换
     * 允许显式构造：Timestamp t(123);
     * 不允许隐式写法：Timestamp t = 123;（会报错）
     * 编码规范：凡是“单参数构造函数”，默认优先加 explicit
     */
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    /**
     * 笔记
     * 虽然now()没有修改成员变量，但是不能添加const修饰函数体
     * 原因是：
     * const成员函数修饰的是隐式this指针
     * 静态成员函数没有this指针，所以语法上不能写成static函数 + const
     */
    static Timestamp now();
    std::string toString() const;

private:
    int64_t microSecondsSinceEpoch_;
};