//
//  Semaphore.h
//  MultiThread
//
//  Created by Borna Noureddin
//  Copyright (c) BCIT. All rights reserved.
//

#ifndef Semaphore_h
#define Semaphore_h

#include <mutex>
#include <condition_variable>
#include <atomic>

class Semaphore
{
public:
    Semaphore(int count_ = 0) : count(count_) {}

    inline void notify()
    {
        //### Use unique_lock to lock a resource
        // We don't necessarily need to lock here since count is atomic
        std::unique_lock<std::mutex> lock(mtx);
        // Increment count, more threads can now enter the semaphore
        count++;
        // Notify a waiting thread that it can enter
        cv.notify_one();
    }

    inline void wait()
    {
        //### Use unique_lock to lock a resource
        // Lock resource following the wait call
        std::unique_lock<std::mutex> lock(mtx);
        // If count is 0, then the semaphore is at capacity and cannot allow any threads in
        while (count == 0)
        {
            // We could also just do nothing instead of doing .wait
            // But that consumes CPU cycles
            // We call .wait to put the thread to sleep until we can wake it up again with notify
            cv.wait(lock);
        }
        // Decrement count if its > 0, this basically means another thread has entered the semaphore
        count--;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> count;
};

#endif
