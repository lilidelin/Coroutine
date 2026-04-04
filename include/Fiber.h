#ifndef _Fiber_H_
#define _Fiber_H_
#include<memory>
#include<functional>
#include<ucontext.h>
#include<atomic>
namespace sylar{
    class Fiber: public std::enable_shared_from_this<Fiber>{
        public:
            enum State{
                RUNNING,
                READY,
                FINISHED,
            };
        private:
            Fiber();
        public:
            Fiber(std::function<void()> func, size_t stack_size = 0, bool run_in_scheduler = true);
            ~Fiber();

            void Reset(std::function<void()> func);
            void Resume();
            void Yield();
            

            uint64_t getId() const {return m_id;}
	        State getState() const {return m_state;}

        public:
            static void SetThis(Fiber* fiber);
            static void MainFun();

            static std::shared_ptr<Fiber> GetThis();

            // 设置调度协程（默认为主协程）
	        static void SetSchedulerFiber(Fiber* f);
	
	        // 得到当前运行的协程id
	        static uint64_t GetFiberId();

        
        private:
            std::function<void()> _func;
            bool m_run_in_scheduler;
            void* m_stack;
            ucontext_t m_ctx;
            std::atomic<State> m_state = State::READY;
            uint64_t m_id;
            uint32_t m_stack_size;
    };

}



#endif