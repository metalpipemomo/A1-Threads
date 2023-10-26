//
//  Created by Borna Noureddin
//  Copyright (c) 2015 BCIT. All rights reserved.
//  COMP 8551 - Deadlocks
//

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <thread>
#include <mutex>
#include <condition_variable>

/// 
/// thread1 and thread2 are started together, using TransferToChequing/Savings.
/// thread1 will first lock the mutex in the savings account and then it sleeps.
/// thread2 in this time will lock the mutex in the chequing account and then sleep
/// When thread1 wakes up, it will try to lock its own mutex (chequing), but will find
/// that the mutex is already owned, so it will wait until the lock is released.
/// When thread2 wakes up, it will try to lock its own mutex (savings), but will find
/// that the mutex is already owned, so it will wait until the lock is released.
/// Both threads are now waiting for the other, causing a deadlock.
/// 

class CAccount
{
public:
    CAccount() { balance = 100; };
    ~CAccount() {};
    void Transfer(CAccount& from, double amount)
    {
        // First thread will lock both mutex' to avoid deadlock, second thread will wait till lock is released
        std::lock(from.m, this->m);
        // We use adopt lock to steal the responsibility for the lock on the from.m mutex and put it on lock_from
        std::lock_guard<std::mutex> lock_from(from.m, std::adopt_lock);
        std::chrono::milliseconds timespan(100);
        std::this_thread::sleep_for(timespan);
        // We use adopt lock to steal the responsibility for the lock on the this->m mutex and put it on lock_to
        std::lock_guard<std::mutex> lock_to(this->m, std::adopt_lock);
        from.balance -= amount;
        this->balance += amount;

        // When the locks go out of scope, they are destroyed and the next thread can lock the mutex'
    }
    double GetBalance() { return balance; };

private:
    std::mutex m;
    double balance;
};

void TransferToChequing(CAccount* chequing, CAccount* savings)
{
    chequing->Transfer(*savings, 10);
}

void TransferToSavings(CAccount* chequing, CAccount* savings)
{
    savings->Transfer(*chequing, 20);
}


//int main(int argc, char* argv[])
//{
//    CAccount chequing, savings;
//    std::cout << "Start potential deadlock actions:\n";
//    std::thread* thread1 = new std::thread(TransferToChequing, &chequing, &savings);
//    std::thread* thread2 = new std::thread(TransferToSavings, &chequing, &savings);
//    thread1->join();
//    thread2->join();
//    std::cout << "\tChq = " << chequing.GetBalance();
//    std::cout << "; Sav = " << savings.GetBalance() << std::endl;
//
//    return 0;
//}