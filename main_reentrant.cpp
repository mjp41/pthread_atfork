#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <mutex>
#include <thread>
#include <sys/wait.h>

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

void notify()
{
  // Make prefork take a long time to encourage a race
  // with a call to pthread_atfork
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main()
{
  std::thread([]{
    // Sleep to ensure the first handler has been registered.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    puts("Register handlers for mutex");
    pthread_atfork(prefork_for_m, postfork_for_m, postfork_for_m); // (a)

    puts("Thread: acquire lock!");
    m.lock();

    //sleep to allow fork to complete while we are here.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    puts("Thread: release lock!");
    m.unlock();
  }).detach();

  puts("Main: pthread_atfork");
  pthread_atfork(notify, nop, nop); // (b)

  puts("Main: fork");
  pid_t pid = fork();

  puts("Main: forked");

  m.lock();

  puts("Main: acquired mutex");

  m.unlock();

  puts("Main: released mutex");

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  if (pid != 0)
  {
    puts("Waiting for child to terminate");
    wait(nullptr);
  }

  puts("Main: terminates");
  return 0;
}