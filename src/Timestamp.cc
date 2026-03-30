#include <time.h>

#include "Timestamp.h"

Timestamp::Timestamp() : secondsSinceEpoch_(0)
{
}

Timestamp::Timestamp(int64_t secondsSinceEpoch)
    : secondsSinceEpoch_(secondsSinceEpoch)
{
}

Timestamp Timestamp::now()
{
    return Timestamp(time(NULL));
}
std::string Timestamp::toString() const
{
    char buf[128] = {0};
    tm *tm_time = localtime(&secondsSinceEpoch_);
    // %4d 最小宽度4，默认用空格补在左边。
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
} 

// #include <iostream>
// int main() {
//     std::cout << Timestamp::now().toString() << std::endl;
//     return 0;
// }