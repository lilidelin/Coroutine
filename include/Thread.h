#ifndef _Thread_H_
#define _Thread_H_

#include<pthread.h>
#include<sys/types.h>
#include<condition_variable>
#include<mutex>
#include<iostream>
#include<string>
#include<functional>

namespace sylar{
    class Semaphore{
        public:
            explicit Semaphore(int count=0):_count(count){}
            void wait(){
                std::unique_lock<std::mutex> lock(_mutex);
                _cv.wait(lock,[&](){
                    return _count>0;
                });
                _count--;
            }
            void signal(){
                std::unique_lock<std::mutex> lock(_mutex);
                _count++;
                _cv.notify_one();
            }

        private:
            std::mutex _mutex;
            std::condition_variable _cv;
            int _count;
    };

    class Thread{
        private:
            pthread_t _tid;
            pid_t _pid;
            std::string _name;
            Semaphore _semaphore;
            std::function<void()> _func;

        public:
            Thread(std::function<void()> func,const std::string& name="");
            ~Thread();

            void join();
            const std::string& getName() const { return _name; }
            pid_t getPid() const { return _pid; }
        
        public:
            static void setName(const std::string& name);
            static const std::string& GetName();
            static pid_t GetPid();
            static Thread* getThis();
        
        private:
            static void* run(void* arg);
    };
}

#endif