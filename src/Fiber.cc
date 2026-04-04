#include"Fiber.h"
#include<iostream>
#include<cassert>

static bool debug = false;
namespace sylar{
// 正在运行的协程
static thread_local Fiber* t_fiber = nullptr;
// 主协程
static thread_local std::shared_ptr<Fiber> t_thread_fiber = nullptr;
// 调度协程
static thread_local Fiber* t_scheduler_fiber = nullptr;
// 协程id
static std::atomic<uint64_t> s_fiber_id{0};
// 协程计数器
static std::atomic<uint64_t> s_fiber_count{0};

Fiber::Fiber(std::function<void()> func, size_t stack_size, bool run_in_scheduler)
:_func(func),m_run_in_scheduler(run_in_scheduler){
    m_state = State::READY;
    m_stack_size = stack_size?stack_size:128000;
    m_stack = malloc(m_stack_size);
    if(getcontext(&m_ctx)){
        std::cerr<<"getcontext failed"<<std::endl;
        pthread_exit(NULL);
    }
    m_ctx.uc_link = NULL;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;
    makecontext(&m_ctx, &Fiber::MainFun,0);
    m_id = s_fiber_id++;
    s_fiber_count++;
    if(debug){
        std::cout<<"Fiber id: "<<m_id<<std::endl;
    }
}

void Fiber::MainFun(){
    std::shared_ptr<Fiber> fiber = GetThis();
    fiber->_func();
    fiber->_func = nullptr;
    fiber->m_state = State::FINISHED;
    
    //auto temp = fiber.get();
    fiber->Yield();
    fiber.reset();
}
void Fiber::Yield(){
    assert(m_state == State::RUNNING||m_state==State::FINISHED);
    if(m_state != State::FINISHED){
        m_state = State::READY;
    }
    if(m_run_in_scheduler){
        SetThis(t_scheduler_fiber);
        if(swapcontext(&m_ctx, &t_scheduler_fiber->m_ctx)){
            std::cerr<<"swapcontext failed"<<std::endl;
            pthread_exit(NULL);
        }
    }
    else{
        SetThis(t_thread_fiber.get());
        if(swapcontext(&m_ctx, &t_thread_fiber->m_ctx)){
            std::cerr<<"swapcontext failed"<<std::endl;
            pthread_exit(NULL);
        }
    }
}
void Fiber::SetThis(Fiber* fiber){
    assert(fiber);
    t_fiber = fiber;
}

std::shared_ptr<Fiber> Fiber::GetThis(){
    if(t_fiber){
        return t_fiber->shared_from_this();
    }
    std::shared_ptr<Fiber> main_fiber(new Fiber());
	t_thread_fiber = main_fiber;
	t_scheduler_fiber = main_fiber.get(); // 除非主动设置 主协程默认为调度协程
	
	assert(t_fiber == main_fiber.get());
	return t_fiber->shared_from_this();
}

Fiber::Fiber(){
    t_fiber = this;
    m_state = State::RUNNING;
    if(getcontext(&m_ctx)){
        std::cerr<<"getcontext failed"<<std::endl;
        pthread_exit(NULL);
    }
    m_id = s_fiber_id++;
    s_fiber_count++;
    if(debug){
        std::cout<<"Fiber id: "<<m_id<<std::endl;
    }
}
uint64_t Fiber::GetFiberId(){
    if(t_fiber){
        return t_fiber->getId();
    }
    return (uint64_t)-1;
}
void Fiber::SetSchedulerFiber(Fiber* f){
    t_scheduler_fiber = f;
}
void Fiber::Reset(std::function<void()> func){
    assert(m_stack!=nullptr&&m_state==State::FINISHED);
    m_state = State::READY;
    _func = func;
    if(getcontext(&m_ctx)){
        std::cerr<<"getcontext failed"<<std::endl;
        pthread_exit(NULL);
    }
    m_ctx.uc_link = NULL;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;
    makecontext(&m_ctx, &Fiber::MainFun,0);
}
void Fiber::Resume(){
    assert(m_state==State::READY);
	
	m_state = State::RUNNING;

	if(m_run_in_scheduler)
	{
		SetThis(this);
		if(swapcontext(&(t_scheduler_fiber->m_ctx), &m_ctx))
		{
			std::cerr << "resume() to t_scheduler_fiber failed\n";
			pthread_exit(NULL);
		}		
	}
	else
	{
		SetThis(this);
		if(swapcontext(&(t_thread_fiber->m_ctx), &m_ctx))
		{
			std::cerr << "resume() to t_thread_fiber failed\n";
			pthread_exit(NULL);
		}	
	}
}
Fiber::~Fiber(){
    s_fiber_count --;
	if(m_stack)
	{
		free(m_stack);
	}
	if(debug) std::cout << "~Fiber(): id = " << m_id << std::endl;
}

}