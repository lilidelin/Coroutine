#include"Thread.h"
#include<sys/syscall.h>
#include<unistd.h>
using namespace sylar;

static thread_local Thread* thread_t;
static thread_local std::string name_t;

Thread::Thread(std::function<void()> func,const std::string& name):_func(func),_name(name){
    pthread_create(&_tid,NULL,run,this);
    if(_tid==0){
        throw std::runtime_error("pthread_create failed");
    }
    _semaphore.wait();
}

Thread::~Thread(){
    if(_tid!=0){
        pthread_detach(_tid);
        _tid=0;
    }
}

void Thread::join(){
    if(_tid!=0){
        int rt = pthread_join(_tid,NULL);
        if(rt!=0){
            throw std::runtime_error("pthread_join failed");
        }
        _tid=0;
    }
}

void* Thread::run(void* arg){
    Thread* thread = (Thread*)arg;
    thread_t=thread;
    setName(thread->_name);
    thread->_pid=GetPid();
    pthread_setname_np(pthread_self(), thread->_name.substr(0, 15).c_str());
    thread->_semaphore.signal();
    thread->_func();
    return NULL;
}

const std::string& Thread::GetName(){
    return name_t;
}

pid_t Thread::GetPid(){
    return syscall(SYS_gettid);
}

Thread* Thread::getThis(){
    return thread_t;
}

void Thread::setName(const std::string& name){
    if(thread_t){
        thread_t->_name=name;
    }
    name_t=name;
}

