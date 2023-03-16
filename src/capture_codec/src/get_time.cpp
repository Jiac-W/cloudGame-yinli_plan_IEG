#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <chrono>
#include <iostream>
#include <sys/time.h>

#include <pthread.h>
#include <unistd.h>


struct str
{
    std::string date;
    std::string msgid;
};

std::time_t getTimeStamp()
{
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = 
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    std::time_t timestamp = tmp.count();
    return timestamp;
}

//获取时间 年-月-日 时-分-秒 毫秒
std::tm* gettm(uint64_t timestamp)
{
    uint64_t milli = timestamp + (uint64_t)8 * 60 * 60 * 1000;
    auto mTime = std::chrono::milliseconds(milli);
    auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
    auto tt = std::chrono::system_clock::to_time_t(tp);
    std::tm* now = std::gmtime(&tt);
    return now;
}

struct str now_str()
{  
    time_t timep;
    timep = getTimeStamp();
    struct tm* info;
    info = gettm(timep);
    char tmp[64];
    char msgid_temp[40];

    timep = timep % 1000000;
    timep = timep % 1000;
    sprintf(tmp, "[%4d-%02d-%02d %02d:%02d:%02d.%03ld]", info->tm_year + 1900, info->tm_mon + 1, info->tm_mday, 
                                                        info->tm_hour, info->tm_min, info->tm_sec, timep);	

    struct str ret_str;
    ret_str.date = tmp;
    ret_str.msgid = msgid_temp;
    return ret_str;
}

// for trans the time of centos cloud 
int main(int argc, char *argv[])
{
    while(1)
    {
        struct str date_ms = now_str();
        printf( "%s\n", date_ms.date.c_str() );
        usleep(1*1000*10); // sleep 10ms
    }
}	
