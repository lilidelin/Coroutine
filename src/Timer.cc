#include"Timer.h"
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
        auto now  = std::chrono::system_clock::now();
        while(!m_timers.empty()&&m_timers.top()->m_next<=now){
            cbs.emplace_back(m_timers.top()->m_cb);
            m_timers.pop();
        }
    }
}