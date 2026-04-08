#ifndef SCHEDULER_H
#define SCHEDULER_H
#include"Fiber.h"
#include"Thread.h"
#include<vector>
#include<mutex>
#include<functional>
#include<memory>
namespace sylar{
struct SchedulerTask{
        std::shared_ptr<Fiber> fiber;
        std::function<void()> callback;
        int thread_id;
        SchedulerTask(std::shared_ptr<Fiber> fiber,int thread_id=-1):fiber(fiber),thread_id(thread_id){}
        SchedulerTask(std::function<void()> callback,int thread_id=-1):callback(callback),thread_id(thread_id){
            fiber = std::make_shared<Fiber>(callback,0,false);
        }
        SchedulerTask(){
            fiber = nullptr;
            callback = nullptr;
            thread_id = -1;
        }
};
class Scheduler{
public:
    Scheduler(int thread_count =1,bool use_caller = false);
    void Run();

    void AddTask(SchedulerTask task);
    void Start();
private:
    int m_thread_count;
    std::vector<Thread*> m_threads;
    std::vector<SchedulerTask> m_tasks;
    bool m_stop {false};
    std::mutex m_mutex;
    bool use_caller;
    std::vector<int> m_threads_id;
    int m_root_thread_id;
    int active_thread_count {0};
};

}
#endif
