#include <atomic>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <mutex>
#include <thread>
#include <iostream>

void nop() {}

std::mutex m;

void prefork_for_m() {
  puts("prefork_for_m");
  m.lock();
  puts("prefork_for_m: lock acquired");
}

void postfork_for_m() {
  puts("postfork_for_m");
  m.unlock();
  puts("postfork_for_m: lock released");
}

int main()
{
//  pthread_atfork(reentrant_prefork, nop, nop);

  std::thread([]{
    puts("Register handlers for mutex");
    pthread_atfork(prefork_for_m, postfork_for_m, postfork_for_m);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    puts("Thread 1: acquire lock!");
    m.lock();

    //sleep for 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    puts("Thread 1: release lock!");
    m.unlock();
  }).detach();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  
  puts("Thread 0: fork");
  fork();

  m.lock();

  m.unlock();

  return 0;
}