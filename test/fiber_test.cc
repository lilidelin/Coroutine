#include"Fiber.h"
#include<vector>
#include<iostream>
using namespace sylar;
class Scheduler{
public:
    void scheduler(std::shared_ptr<Fiber> fiber){
        fibers.push_back(fiber);
    }
    void run(){
        std::cout<<"membr"<<fibers.size()<<std::endl;
        for(auto& fiber: fibers){
            fiber->Resume();
        }
        fibers.clear();
    }
private:
    std::vector<std::shared_ptr<Fiber>> fibers;
};

void fiber_func(int i){
    std::cout<<"fiber_func"<<i<<std::endl;
}

int main(){
    Fiber::GetThis();
    Scheduler sc;
    for(int i=0;i<20;i++){
        sc.scheduler(std::make_shared<Fiber>(std::bind(&fiber_func,i),0,false));
    }
    sc.run();
    return 0;
}
