#ifndef __STAT_H__
#define __STAT_H__

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <thread>

#include "log.h"

using namespace std;

extern int parseLine(char *line);

extern int getValue();

inline std::string exec(const char *cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

inline std::string dstat() {
    std::array<char, 512> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen("dstat -tcdrlmgy --fs 1 105", "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    int times = 0;
    int global_times = 0;
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 512, pipe.get()) != nullptr) {
            if (global_times < 100) {
                result += buffer.data();
                times++;
                global_times++;
                if (times >= 5) {
                    log_debug(
                            "\n----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg---"
                            " ------memory-usage----- ---paging-- ---system-- --filesystem-"
                            "\n     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | "
                            "used  buff  cach  free|  in   out | int   csw |files  inodes"
                            "\n%s", result.c_str());
                    times = 0;
                    result.clear();
                }
            } else {
                log_debug(
                        "\n----system---- --total-cpu-usage-- -dsk/total- --io/total- ---load-avg---"
                        " ------memory-usage----- ---paging-- ---system-- --filesystem-"
                        "\n     time     |usr sys idl wai hiq siq| read  writ| read  writ| 1m   5m  15m | "
                        "used  buff  cach  free|  in   out | int   csw |files  inodes"
                        "\n%s", buffer.data());
            }
        }
    }
    return result;
}

inline void DstatThreading() {
    thread t = thread([]() {
        dstat();
    });
    t.detach();
}

#endif

