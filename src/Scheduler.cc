#include"Scheduler.h"
#include<iostream>
static bool debug = true;
namespace sylar{
Scheduler::Scheduler(int thread_count,bool use_caller):use_caller(use_caller),m_thread_count(thread_count){
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
    m_threads.resize(m_thread_count);
    for(int i=0; i< m_thread_count; i++){
        m_threads[i] = new Thread(std::bind(&Scheduler::Run,this),"thread"+std::to_string(i));
        m_threads_id.push_back(m_threads[i]->getPid());
    }
    if(debug){
        std::cout<<"Scheduler start, root thread id: "<<m_root_thread_id<<std::endl;
    }
}
void Scheduler::Run(){
    int cur_thread_id = Thread::GetPid();
    if(debug){
        std::cout<<"Thread "<<Thread::GetName()<<std::endl;
    }
    if(cur_thread_id!=m_root_thread_id){
        Fiber::GetThis();
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
            task.fiber->Resume();
            active_thread_count--;
        }
    }
}
void Scheduler::AddTask(SchedulerTask task){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_tasks.push_back(task);
}
}
