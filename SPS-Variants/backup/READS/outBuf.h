#ifndef __OUTBUF_H__
#define __OUTBUF_H__

#include <cstring>
#include <cstdio>

struct outBuf {
    const static int bufSize = 64 * 1024;
    int cnt, buf[bufSize];
    FILE *fg;

    outBuf(char *fName) {
        fg = fopen(fName, "wb");
        cnt = 0;
    }

    ~outBuf() {
        fwrite(buf, sizeof(int), cnt, fg);
        fclose(fg);
    }

    void insert(int x) {
        if (cnt == bufSize) {
            fwrite(buf, sizeof(int), bufSize, fg);
            cnt = 0;
        }
        buf[cnt++] = x;
    }

    void insertLL(long long x) {
        if (cnt == bufSize) {
            fwrite(buf, sizeof(int), bufSize, fg);
            cnt = 0;
        }
        buf[cnt++] = (x >> 32);
        if (cnt == bufSize) {
            fwrite(buf, sizeof(int), bufSize, fg);
            cnt = 0;
        }
        buf[cnt++] = ((x << 32) >> 32);

    }

};


#endif