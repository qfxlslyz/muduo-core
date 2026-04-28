#!/bin/bash

set -e

# 如果没有build目录，创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build && \
    cmake .. && \
    make

# 回到项目根目录
cd ..

# 把头文件拷贝到 /usr/include/muduo_core  so库拷贝到 /usr/lib  PATH
if [ ! -d /usr/include/muduo_core ]; then
    mkdir /usr/include/muduo_core
fi

for header in `ls include/*.h`
do
    cp $header /usr/include/muduo_core
done

cp `pwd`/lib/libmuduo_core.so /usr/lib

# 更新动态链接库缓存
ldconfig