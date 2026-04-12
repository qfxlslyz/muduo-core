# 创建可执行文件
add_executable(testserver testserver.cc)
add_executable(echo_bench echo_bench.cc)

# 链接必要的库，比如刚刚我们写好的在 src 文件 CMakeLists 中 muduo-core_lib 静态库，还有全局链接库
target_link_libraries(testserver muduo_core ${LIBS})
target_link_libraries(echo_bench muduo_core ${LIBS})

# 设置编译选项
target_compile_options(testserver PRIVATE -std=c++11 -Wall)
target_compile_options(echo_bench PRIVATE -std=c++11 -Wall)

# 设置 testserver 可执行文件输出目录
set_target_properties(testserver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(echo_bench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
