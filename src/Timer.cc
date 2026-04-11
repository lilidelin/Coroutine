#include"Timer.h"
#include<iostream>
namespace sylar{
    Timer::Timer(std::function<void()>cb,uint64_t ms):m_cb(cb),m_ms(ms){
        m_next = std::chrono::system_clock::now() + std::chrono::milliseconds(m_ms);
    }
    Timer::~Timer(){
        m_cb = nullptr;
        m_ms = 0;
        m_next = std::chrono::time_point<std::chrono::system_clock>();
    }
    TimerManager::TimerManager(){
        
    }
    void TimerManager::addTimer(Timer* timer){
        m_timers.push(timer);
    }
    void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs){
        std::lock_guard<std::mutex> lock(m_mutex);
        auto now  = std::chrono::system_clock::now();
        while(!m_timers.empty()&&m_timers.top()->m_next<=now){
            cbs.emplace_back(m_timers.top()->m_cb);
            m_timers.pop();
        }
    }   
    uint64_t TimerManager::getnextTimer(){
        //std::cout<<"getnextTimer"<<std::endl;
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_timers.empty()){
            return ~0ull;
        }
        auto now = std::chrono::system_clock::now();
        if(now>=m_timers.top()->m_next){
            return 0;
        }
        else{
            auto res = std::chrono::duration_cast<std::chrono::milliseconds>(m_timers.top()->m_next - now);
            return static_cast<uint64_t>(res.count());
        }
    }
}