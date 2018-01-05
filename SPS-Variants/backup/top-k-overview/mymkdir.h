#pragma once

#include <sys/stat.h>

#include <cerrno>

#include <iostream>
#include <string>

namespace light {
    int mkpath(std::string s, mode_t mode = 0755) {
        size_t pre = 0, pos;
        std::string dir;
        int mdret;

        if (s[s.size() - 1] != '/') {
            s += '/';
        }

        while ((pos = s.find_first_of('/', pre)) != std::string::npos) {
            dir = s.substr(0, pos++);
            pre = pos;
            if (dir.size() == 0) continue; // if leading / first time is 0 length
            if ((mdret = ::mkdir(dir.c_str(), mode)) && errno != EEXIST) {
                return mdret;
            }
        }
        return mdret;
    }

}
