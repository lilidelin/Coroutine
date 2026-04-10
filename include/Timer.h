#ifndef __TIMER_H__
#define __TIMER_H__
#include<functional>
#include<vector>
#include<queue>
#include<chrono>
#include<cstdint>
namespace sylar{
    class Timer{
    public:
        Timer(std::function<void()>cb,uint64_t ms=0);
        ~Timer();
    public:
        std::function<void()> m_cb;
        uint64_t m_ms;
        std::chrono::time_point<std::chrono::system_clock> m_next;
    };
    class TimerManager{
        public:
            TimerManager();
            void addTimer(Timer* timer);
            void listExpiredCb(std::vector<std::function<void()>>& cbs);

        private:
            struct Comparator{
                bool operator()(Timer* a, Timer* b){
                    return a->m_next > b->m_next;
                }
            };
            std::priority_queue<Timer*,std::vector<Timer*>,Comparator> m_timers;
    };
}



#endif