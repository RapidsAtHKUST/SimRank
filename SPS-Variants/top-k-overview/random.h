/*
 * Author: Shao Yingxia
 * Create Date: 2012年12月22日 星期六 21时39分18秒
 */
#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <cstdio>
#include <cstdlib>
#include <ctime>

class Random {
public:
    Random() {
        srand(time(NULL));
//            fp = fopen("test/random.num","r");
    }

    ~Random() {
//            if(fp != NULL) fclose(fp);
    }

    int getRandom() {
//            if(fp && !feof(fp)){
//                int num;
//                fscanf(fp, "%d",&num);
//                return num;
//            }
        return rand();
    }
//    private:
//        FILE* fp;
};

#endif    // #ifndef __RANDOM_H__

