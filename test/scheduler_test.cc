#include"Scheduler.h"
#include<iostream>
#include<unistd.h>
using namespace sylar;
void taskfunc(int id){
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
    sleep(10);
    return 0;
}