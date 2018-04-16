#ifndef __ADDRARRAY_HPP__
#define __ADDRARRAY_HPP__    1

#include <pthread.h>

#include "Config.hpp"

class AddrArray {
public:
    pthread_mutex_t mLock;
    unsigned int mCurrentIndex;

public:

    AddrArray() {
        pthread_mutex_init(&mLock, NULL);
        mCurrentIndex = MINNET;
    }

    unsigned int GetNextArrayIndex() {
        pthread_mutex_lock(&mLock);

        if (mCurrentIndex >= MAXNET) {
            pthread_mutex_unlock(&mLock);
            return 0;
        } else if (mCurrentIndex == 10) {
            mCurrentIndex++;
        }

        unsigned int ret = mCurrentIndex;

        mCurrentIndex++;
        pthread_mutex_unlock(&mLock);

        return ret;
    }
};

#endif // __ADDRARRAY_HPP__
