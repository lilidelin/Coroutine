#include"IOManager.h"
#include<iostream>
#include<unistd.h>
#include<sys/epoll.h>
namespace sylar{
    static bool debug = false;
    IOManager::IOManager(int thread_count):Scheduler(thread_count),TimerManager(){
        m_fdctx.resize(1024);
        int epfd = epoll_create(1024);
        if(epfd<0){
            std::cout<<"epoll_create error"<<std::endl;
            return;
        }
        if(debug){
            std::cout<<"IOManager start, epfd: "<<epfd<<std::endl;
        }
        m_epfd = epfd;
        // 在 Start() 之前设置 this，确保主线程调用 addEvent 时能获取到正确的 scheduler
        Scheduler::SetThis(this);
        Start();
    }
    IOManager::~IOManager(){
        
    }
    //run是线程调用的函数，用于从任务队列取任务执行和调用idle处理定时任务和io事件
    void IOManager::Run(){
        SetThis(this);
        if(Thread::GetPid()!=m_root_thread_id){
            Fiber::GetThis();
            if(debug) std::cout<<"Thread "<<Thread::GetName()<<" mian fiber id:"<<Fiber::GetFiberId()<<std::endl;
        }
        std::shared_ptr<Fiber> idle_fiber = std::make_shared<Fiber>(std::bind(&IOManager::idle,this),0,false);
        while(true){
            SchedulerTask task;
            {
                std::lock_guard<std::mutex> lock(Scheduler::m_mutex);
                auto it = m_tasks.begin();
                while(it != m_tasks.end()){
                    if(it->thread_id!=Thread::GetPid()&&it->thread_id!=-1){
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
                //这里是否需要一个判断来判断任务是否已完成呢？
                //如果任务已完成，那么就不需要再执行了
                if(task.fiber->getState()!=Fiber::FINISHED){
                    task.fiber->Resume();
                }
                active_thread_count--;
            }
            //else说明没有任务了，这个时候我要切换到idle协程去运行
            else{
                if(idle_fiber->getState()==Fiber::FINISHED){
                    break;
                }
                idle_thread_count++;
                idle_fiber->Resume();
                idle_thread_count--;
            }
            if(stopping()){
                break;
            }
        }
    }
    //idle协程用于处理定时任务和io事件
    void IOManager::idle(){
        epoll_event events[1024];
        while(true){
            //先来处理io事件，这里负责取出io事件，需要用到epoll_wait来把已经响应的io事件放入就绪队列中，接口如下
            //int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);
            //这个epfd应该要在构造函数中创建
            //events在循环外创建
            //最大事件设置为event的大小
            //timeout时间要设置为当前最快超时的定时任务时间
            if(debug){
                std::cout<<"IOManager idle, Run in Thread : "<<Thread::GetPid()<<std::endl;
            }
            int ret = 0;
            while(true){
                uint64_t maxtimeout = 5000;
                uint64_t timeout = getnextTimer();
                timeout = std::min(timeout,maxtimeout);
                if(debug){
                    std::cout << "m_epfd = " << m_epfd << std::endl;
                    std::cout << "timeout = " << timeout << std::endl;
                    std::cout << "events array address = " << events << std::endl;  
                }
                ret = epoll_wait(m_epfd,events,1024,timeout);
                if(debug){
                    std::cout<<"epoll_wait ret: "<<ret<<std::endl;
                }
                if(ret<0&&errno==EINTR){
                    continue;
                }
                else{
                    if(debug){
                        std::cout<<"epoll_wait timeout: "<<timeout<<std::endl;
                    }
                    break;
                }
            }
            //先添加超时任务
            std::vector<std::function<void()>> cbs;
            listExpiredCb(cbs);
            if(!cbs.empty()){
                std::lock_guard<std::mutex> lock(Scheduler::m_mutex);
                for(auto cb:cbs){
                    m_tasks.emplace_back(SchedulerTask(cb));
                }
                cbs.clear();
            }
            if(debug){
                std::cout<<"success enter io event "<<"io sum:"<<ret<<std::endl;
            }
            //处理io事件
            for(int i=0;i<ret;i++){
                //std::lock_guard<std::mutex> lock(write_mtx);
                int fd = events[i].data.fd;
                //std::cout<<"idle: m_fdctx.size()="<<m_fdctx.size()<<" fd="<<fd<<std::endl;
                FdContext* ctx = nullptr;
                if(fd < m_fdctx.size()){
                    ctx = m_fdctx[fd];
                }
                //std::cout<<"idle: ctx="<<ctx<<std::endl;
                if(ctx==nullptr){
                    std::cout<<"fd "<<fd<<" not found in m_fdctx"<<std::endl;
                    continue;
                }
                if(events[i].events&EPOLLIN){
                    if(debug){
                        std::cout<<"EPOLLIN event fd = "<<fd<<std::endl;
                    }
                    //处理读事件
                    ctx->triggerEvent(READ);
                }
                if(events[i].events&EPOLLOUT){
                    if(debug){
                        std::cout<<"EPOLLOUT event fd = "<<fd<<std::endl;
                    }
                    //处理写事件
                    ctx->triggerEvent(WRITE);
                }
            }
            Fiber::GetThis()->Yield();
        }
    }

    int IOManager::addEvent(int fd,EventType type,std::function<void()> cb){
        //这个函数用来添加io事件，添加io事件的上下文到数组中
        //用epoll_ctl来添加事件，在这之前应该先判断事件的类型
        FdContext* ctx = nullptr;
        std::lock_guard<std::mutex> lock(write_mtx);
        if(fd>=m_fdctx.size()){
            m_fdctx.resize(fd*1.5);
        }
        ctx = m_fdctx[fd];
        //std::cout<<"addEvent: m_fdctx["<<fd<<"] address = "<<m_fdctx[fd]<<std::endl;
        if(ctx == nullptr){
            ctx = new FdContext();
            m_fdctx[fd] = ctx;
        }
        if(ctx->event_types&type){
            return -1;
        }
        int op = ctx->event_types?EPOLL_CTL_MOD:EPOLL_CTL_ADD;
        epoll_event ev;
        ev.data.fd = fd;
        ev.events = type|ctx->event_types|EPOLLET;
        //if(debug){
        //    std::cout<<"add event fd = "<<fd<<", type = "<<type<<", ctx->event_types = "<<ctx->event_types<<std::endl;
        //}
        int rt = epoll_ctl(m_epfd,op,fd,&ev);
        if(rt<0){
            std::cout<<"add event failed: "<<errno<<std::endl;
            return -1;
        }
        ctx->event_types = (EventType)(type|ctx->event_types);
        FdContext::EventContext& ec = ctx->GetEventContext(type);  // 改为引用！
        //std::cout<<"addEvent: ctx="<<ctx<<" &write_ctx="<<&ctx->write_ctx<<" &read_ctx="<<&ctx->read_ctx<<std::endl;
        // 强制使用 this，不管 GetThis() 返回什么
        //std::cout<<"addEvent: setting ec.scheduler="<<this<<" for type="<<type<<std::endl;
        Scheduler::SetThis(this);
        ec.scheduler = this;
        if(cb){
            ec.cb.swap(cb);
        }
        else{
            ec.fiber = Fiber::GetThis();
            assert(ec.fiber->getState()==Fiber::RUNNING);
        }
        return 0;
}
}
