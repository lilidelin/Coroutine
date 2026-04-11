#include"Scheduler.h"
#include<iostream>
static bool debug = false;
namespace sylar{
static thread_local Scheduler* m_this = nullptr;
Scheduler::Scheduler(int thread_count,bool use_caller):use_caller(use_caller),m_thread_count(thread_count),m_root_thread_id(0){
    //SetThis(this);  // 移除这里，在Start()中调用
    if(use_caller){
        Fiber::GetThis();
        m_root_thread_id = Thread::GetPid();
        if(debug){
            std::cout<<"Scheduler root thread id: "<<m_root_thread_id<<std::endl;
        }
        m_threads_id.push_back(m_root_thread_id);
        m_thread_count--;
    }
}
void Scheduler::Start(){
    SetThis(this);
    m_threads.resize(m_thread_count);
    for(int i=0; i< m_thread_count; i++){
        m_threads[i] = new Thread(std::bind(&Scheduler::Run,this),"thread"+std::to_string(i));
        m_threads_id.push_back(m_threads[i]->getPid());
    }
    if(debug){
        std::cout<<"Scheduler start, root thread id: "<<m_root_thread_id<<std::endl;
    }
}
bool Scheduler::stopping(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_stop&&active_thread_count==0&&m_tasks.empty();
}
void Scheduler::Stop(){
    if(stopping()){
        return;
    }
    m_stop = true;
    for(auto& th:m_threads){
        th->join();
    }
}
void Scheduler::Run(){
    SetThis(this);
    int cur_thread_id = Thread::GetPid();
    if(debug){
        std::cout<<"Thread "<<Thread::GetName()<<std::endl;
    }
    if(cur_thread_id!=m_root_thread_id){
        Fiber::GetThis();
        if(debug) std::cout<<"thread mian fiber id:"<<Fiber::GetFiberId()<<std::endl;
    }
    while(true){
        SchedulerTask task;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_tasks.begin();
            while(it!=m_tasks.end()){
                if(it->thread_id!=cur_thread_id&&it->thread_id!=-1){
                    it++;
                    continue;
                }
                task = *it;
                m_tasks.erase(it);
                active_thread_count++;
                break;
            }
        }
        if(task.fiber!=nullptr){
            if(task.fiber->getState()!=Fiber::FINISHED){
                task.fiber->Resume();
            }
            active_thread_count--;
        }
        if(stopping()){
            break;
        }
    }
}
void Scheduler::AddTask(SchedulerTask task){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_tasks.push_back(task);
    if(debug){
        std::cout<<"AddTask, thread id: "<<task.thread_id<<std::endl;
    }
}
Scheduler* Scheduler::GetThis(){
    return m_this;
}
void Scheduler::idle(){
    
}

void Scheduler::SetThis(Scheduler* scheduler){
    m_this = scheduler;
}


}
