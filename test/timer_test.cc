#include"Timer.h"
#include<iostream>
#include<unistd.h>
using namespace sylar;
void fun(int i){
    std::cout<<"fun "<<i<<std::endl;
}
int main(){
    TimerManager tm;
    for(int i=0;i<10;i++){
        tm.addTimer(new Timer(std::bind(fun,i),i*1000));
    }
    std::vector<std::function<void()>> cbs;
    sleep(5);
    tm.listExpiredCb(cbs);
    for(auto cb:cbs){
        cb();
    }
    sleep(5);
    tm.listExpiredCb(cbs);
    for(auto cb:cbs){
        cb();
    }
    return 0;
}