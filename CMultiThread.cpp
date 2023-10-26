//
//  CMultiThread.cpp
//  MultiThread
//
//  Created by Borna Noureddin
//  Copyright (c) BCIT. All rights reserved.
//

#include "CMultiThread.h"
#include "Semaphore.h"

//### Define mutexes and semaphores
std::mutex mtx;
std::condition_variable cv;
bool cv_ready = false;
// Initialize reader semaphore with 0
// No thread can enter it initially because there is nothing to read initially
Semaphore reader{0};
// Initialize writer to total number of characters in a buffer
// So we can allow as many threads to enter the writing function
// as the buffer allows
Semaphore writer{NUM_TOTAL_BUFFERS};

CMultiThread::CMultiThread()
{
    th1 = th2 = NULL;
}

CMultiThread::~CMultiThread()
{
    if (th1) delete th1;
    if (th2) delete th2;
}

double CMultiThread::UniformRandom()
{
    double r = rand();
    r /= (double)RAND_MAX;
    return r;
}

int CMultiThread::RandomInt(int minVal, int maxVal)
{
    double r = UniformRandom();
    return (int)(r * (maxVal - minVal) + minVal);
}



////////////////////////////////////////////////////////////////////////
//
// Two thread synchronization (single resource)
//

void CMultiThread::TwoThreadTestWorkerThread(CMultiThread* th, int n, char c, int sleepTime)
{
    std::chrono::milliseconds timespan(sleepTime);
    std::this_thread::sleep_for(timespan);

    //### This function will need some sort of synchronization...
    // Lock the following code while a thread is inside of it
    mtx.lock();
    for (int i = 0; i < n; ++i) {std::cout << c;}
    std::cout << '\n';
    // Unlock the previous code to allow another thread to enter
    mtx.unlock();
}

void CMultiThread::TwoThreadTest()
{
    th1 = new std::thread(TwoThreadTestWorkerThread, this, 500, '*', 0);
    th2 = new std::thread(TwoThreadTestWorkerThread, this, 500, '$', 0);
    th1->join();
    th2->join();
    std::cout << "Completed two thread example!" << std::endl;
}



////////////////////////////////////////////////////////////////////////
//
// Multiple thread synchronization (single resource)
//

void CMultiThread::MultiThreadTestWorkerThread(CMultiThread* th, int num, int max)
{
    //### This function will need some sort of synchronization...
    // Create a unique lock
    std::unique_lock<std::mutex> lock(mtx);
    // Wait for a thread whos ID matches the ID given to the function and also see wait for ready signal
    // This lines up the thread execution according to their creation order
    // Probably don't need to do this
    cv.wait(lock, [th, num] { return th->GetCurrentThreadId() == num && cv_ready; });

    th->IncrementCurrentThreadId();

    std::cout << "Thread: ";
    std::cout << num + 1 << " / " << max;
    std::cout << " current count is: ";

    std::cout << th->GetCurrentThreadId() << std::endl;

    //### Remember to clean up synchronization...
    // Notify all waiting threads
    // If we only notify one and the cv condition doesn't pass, program will deadlock
    cv.notify_all();
}

void CMultiThread::MultiThreadTest()
{
    std::thread* threads = new std::thread[numThreads];

    for (int id = 0; id < numThreads; id++)
        threads[id] = std::thread(MultiThreadTestWorkerThread, this, id, numThreads);

    std::cout << "\nRunning " << numThreads;
    std::cout << " in parallel: \n" << std::endl;

    //### This function will need some sort of synchronization...
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv_ready = true;
        cv.notify_all();
    }
    for (int id = 0; id < numThreads; id++)
    {
        threads[id].join();
    }

    std::cout << "\nCompleted multiple threads example!\n";
    std::cout << std::endl;

    delete[] threads;
    current = 0;
}

void CMultiThread::IncrementCurrentThreadId()
{
    current++;
}

int CMultiThread::GetCurrentThreadId()
{
    return current;
}



////////////////////////////////////////////////////////////////////////
//
// Reader-Writer synchronization
//

//  Reader-Writer example
//
// In this classic Reader-Writer problem, there are two threads exchanging
// information through a fixed size buffer. The Writer thread fills the buffer
// with data whenever there's room for more. The Reader thread reads data from
// the buffer and prints it. Both threads have a situation where they should
// block. The writer blocks when the buffer is full and the reader blocks when
// the buffer is empty. The problem is to get them to cooperate nicely and
// block efficiently when necessary.
// Multiple runs will not necessary get the same output, but the reader
// should always list the chars in the same order as the writer and there
// should never be more than 5 writes before a read and vice versa (since
// we only have 5 buffers to use).

void CMultiThread::ReaderWriterTest()
{
    //### This function will need some sort of synchronization...
    // I don't think you need to add anything here
    std::thread* writerThread = new std::thread(Writer, this);
    std::thread* readerThread = new std::thread(Reader, this);
    writerThread->join();
    readerThread->join();
    std::cout << "\nCompleted Reader-Writer example!\n";
    std::cout << std::endl;
}

void CMultiThread::Writer(CMultiThread* th)
{
    int i, writePt = 0;
    char data;
    //### This function will need some sort of synchronization in the loop...
    for (i = 0; i < DATA_LENGTH; i++)
    {
        // Make writing threads wait in case there is no room in the buffer to write
        writer.wait();

        data = th->PrepareData();
        th->buffers[writePt] = data;
        printf("Writer: buffer[%d] = %c\n", writePt, data);
        writePt = (writePt + 1) % NUM_TOTAL_BUFFERS;

        // Tell the reader that it can allow a thread in to read now that something has been written
        reader.notify();
    }
}

void CMultiThread::Reader(CMultiThread* th)
{
    int i, readPt = 0;
    char data;
    //### This function will need some sort of synchronization in the loop...
    for (i = 0; i < DATA_LENGTH; i++)
    {
        // Make reading threads wait till it is told that there is something to read
        reader.wait();

        data = th->buffers[readPt];
        printf("\t\tReader: buffer[%d] = %c\n", readPt, data);
        readPt = (readPt + 1) % NUM_TOTAL_BUFFERS;
        th->ProcessData(data);
        
        // Tell the writer that it can now write again as the buffer has an empty slot
        writer.notify();
    }
}

void CMultiThread::ProcessData(char data)
{
    std::chrono::milliseconds timespan(RandomInt(0, 500));
    std::this_thread::sleep_for(timespan);
}

char CMultiThread::PrepareData(void)
{
    std::chrono::milliseconds timespan(RandomInt(0, 500));
    std::this_thread::sleep_for(timespan);
    return (char)RandomInt('A', 'Z');
}
