#ifndef IO_MANAGER_H
#define IO_MANAGER_H
#include"Timer.h"
#include"Scheduler.h"
#include"Thread.h"
#include"Fiber.h"
#include<vector>
#include<functional>
#include<memory>
#include<assert.h>
#include<mutex>
namespace sylar{
class IOManager:public TimerManager,public Scheduler{
public:
    enum EventType{
        NONE = 0x0,
        //READ == EPOLLIN 
        READ = 0x1,
        //WRITE == EPOLLOUT
        WRITE = 0x4,
    };
    struct FdContext{
        struct EventContext{
            std::function<void()> cb;
            std::shared_ptr<Fiber> fiber;
            Scheduler* scheduler;
        };
        EventContext read_ctx;
        EventContext write_ctx;
        EventType event_types = NONE;
        int fd = 0;

        EventContext& GetEventContext(EventType type){
            assert(type==READ||type==WRITE);
            if(type==READ){
                return read_ctx;
            }
            if(type==WRITE){
                return write_ctx;
            }
            throw std::runtime_error("GetEventContext error");
        }
        void resetEventContext(EventContext& ctx){
            ctx.cb = nullptr;
            ctx.fiber = nullptr;
            ctx.scheduler = nullptr;
        }
        void triggerEvent(EventType type){
            // 检查该类型事件是否存在（可能被多次触发）
            if(!(event_types & type)){
                return;  // 该类型事件已被处理，跳过
            }
            event_types = (EventType)(event_types&~type);//这个事件触发了，通过这种方法除去类型
            EventContext& ctx = GetEventContext(type);
            //std::cout<<"triggerEvent: ctx.scheduler="<<ctx.scheduler<<" this="<<this<<std::endl;
            if(ctx.fiber){
                ctx.scheduler->AddTask(SchedulerTask(ctx.fiber));
            }
            else{
                ctx.scheduler->AddTask(SchedulerTask(ctx.cb));
            }
            resetEventContext(ctx);
        }
    };
public:
    IOManager(int thread_count);
    ~IOManager();
    void Run() override;
    void idle() override;
    int addEvent(int fd,EventType type,std::function<void()> cb);

private:
    int m_epfd;
    std::vector<FdContext*> m_fdctx;
    std::mutex read_mtx;
    std::mutex write_mtx;
};
}
#endif
