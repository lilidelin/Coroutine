#include"Scheduler.h"
#include<iostream>
#include<unistd.h>
#include<mutex>
std::mutex mutex_cout;
using namespace sylar;
void taskfunc(int id){
    std::lock_guard<std::mutex> lock(mutex_cout);
    std::cout<<"taskfunc id: "<<id<<std::endl;
}
int main(){
    Scheduler scheduler(3,true);
    scheduler.Start();
    for(int i = 0;i<10;i++){
        std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>(std::bind(&taskfunc,i),0,false);
        scheduler.AddTask(SchedulerTask(fiber));
    }
    sleep(2);
    for(int i = 10;i<16;i++){
        std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>(std::bind(&taskfunc,i),0,false);
        scheduler.AddTask(SchedulerTask(fiber));
    }
    scheduler.Stop();
    return 0;
}