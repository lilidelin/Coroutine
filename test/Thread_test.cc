#include"Thread.h"
#include<iostream>
#include<memory>
#include<vector>
using namespace sylar;
void fun(){
    std::cout<<"run thread name="<<Thread::GetName()<<" pid="<<Thread::GetPid()<<std::endl;
}
int main(){
    int n=10;
    std::vector<std::unique_ptr<Thread>> threads(n);
    for(int i=0;i<n;i++){
        threads[i]=std::make_unique<Thread>(fun,"thread_"+std::to_string(i));
        std::cout<<"create thread "<<threads[i]->getName()<<" pid "<<threads[i]->getPid()<<std::endl;
    }
    for(int i=0;i<n;i++){
        threads[i]->join();
    }
}
