/*
 * Author: Shao Yingxia
 * Create Date: 2012年12月12日 星期三 22时55分34秒
 */
#ifndef __MYQUEUE_HPP__
#define __MYQUEUE_HPP__

class MyQueue {
public:
    MyQueue() : array(NULL), maxSize(0), delta(100), head(0), tail(0) {}

    MyQueue(int size) : maxSize(size), delta(size), head(0), tail(0) {
        array = (int *) malloc(sizeof(int) * size);
    }

    ~MyQueue() { free(array); }

    /* queue interface */
    int front() {
        if (!empty())
            return array[head];
        return -1;
    }

    int pop() {
        head++;
        return array[head - 1];
    }

    void push(int val) {
        if (tail == maxSize) {
            int *nptr = (int *) realloc(array, sizeof(int) * (maxSize + delta));
            maxSize = (nptr == NULL ? maxSize : maxSize + delta);
            array = (nptr == NULL ? array : nptr);
        }
        array[tail++] = val;
    }

    bool empty() { return head >= tail; }

    /* array interface */
    int getVal(int idx) { return array[idx]; }

    int *getContent() { return array; }

    int length() { return tail; }

private:
    int *array;
    int maxSize;
    int delta;
    int head;
    int tail;
};

#endif
