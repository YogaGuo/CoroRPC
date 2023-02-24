/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-10 11:13:09
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-21 18:30:10
 */
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <atomic>
#include "compactrpc/coroutine/coroutine.h"
#include "compactrpc/comm/log.h"
#include "compactrpc/comm/run_time.h"

namespace compactrpc
{
    // 每一个IO线程都要有一个主协程， 主协程的栈就是当前线程的栈
    static thread_local Coroutine *t_main_coroutine = nullptr;

    // 当前正在执行的协程
    static thread_local Coroutine *t_cur_coroutine = nullptr;

    static thread_local RunTime *t_cur_run_time = nullptr;

    static std::atomic_int t_cur_coroutine_count{0};

    static std::atomic_int t_cur_coroutine_id{0};

    int getCoroutineIndex()
    {
        return t_cur_coroutine_id;
    }

    void setCurrentRuntime(RunTime *v)
    {
        t_cur_run_time = v;
    }
    RunTime *getCurrentRunTime()
    {
        return t_cur_run_time;
    }
    void CoFunction(Coroutine *co)
    {
        if (co)
        {
            co->setIsInCorFunc(true);

            // exec callback_func;
            co->m_call_back();

            co->setIsInCorFunc(false);
        }
        // cor function's callback finished, that means cor's life is over, we should yield main cor
        Coroutine::Yield();
    }

    Coroutine::Coroutine()
    {
        m_cor_id = 0;
        t_cur_coroutine_count++;
        memset(&m_coctx, 0, sizeof(m_coctx));
        t_cur_coroutine = this;
    }

    Coroutine::Coroutine(int size, char *stack_ptr) : m_stack_size(size), m_stack_sp(stack_ptr)
    {
        assert(stack_ptr);
        if (!t_main_coroutine)
            t_main_coroutine = new Coroutine();

        m_cor_id = t_cur_coroutine_id++;
        t_cur_coroutine_count++;
    }

    Coroutine::Coroutine(int size, char *stack_ptr, std::function<void()> cb)
        : m_stack_size(size), m_stack_sp(stack_ptr)
    {
        assert(m_stack_sp);
        if (!t_main_coroutine)
            t_main_coroutine = new Coroutine();
        setCallBack(cb);
        m_cor_id = t_cur_coroutine_id++;
        t_cur_coroutine_count++;
    }

    bool Coroutine::setCallBack(std::function<void()> cb)
    {
        if (this == t_main_coroutine)
        {
            ErrorLog << "main coroutine can't set callback";
            return false;
        }
        if (m_is_in_cofunc)
        {
            ErrorLog << "this coroutine is in Cofunction";
            return false;
        }
        m_call_back = cb;

        char *top = m_stack_sp + m_stack_size;

        top = reinterpret_cast<char *>((reinterpret_cast<unsigned long>(top)) & -16LL);

        memset(&m_coctx, 0, sizeof(m_coctx));

        m_coctx.regs[KRSP] = top;
        m_coctx.regs[KRBP] = top;
        m_coctx.regs[KRETAddr] = reinterpret_cast<char *>(CoFunction);
        m_coctx.regs[KRDI] = reinterpret_cast<char *>(this);
        m_can_resume = true;
        return true;
    }

    Coroutine::~Coroutine()
    {
        t_cur_coroutine_count--;
    }

    Coroutine *Coroutine::GetCurrentCoroutine()
    {
        if (!t_cur_coroutine)
        {
            t_main_coroutine = new Coroutine();
            t_cur_coroutine = t_main_coroutine;
        }
        return t_cur_coroutine;
    }

    Coroutine *Coroutine::GetMainCoroutine()
    {
        if (t_main_coroutine)
            return t_main_coroutine;
        t_main_coroutine = new Coroutine();
        return t_main_coroutine;
    }

    bool Coroutine::IsMainCoroutine()
    {
        if (!t_main_coroutine || t_cur_coroutine == t_main_coroutine)
            return true;
        return false;
    }

    // form taget cor back to main cor
    void Coroutine::Yield()
    {

        if (t_main_coroutine == nullptr)
        {
            ErrorLog << "main cor is nullptr";
            return;
        }

        if (t_cur_coroutine == t_main_coroutine)
        {
            ErrorLog << "current cor is in main cor";
            return;
        }

        Coroutine *co = t_cur_coroutine;
        t_cur_coroutine = t_main_coroutine;
        t_cur_run_time = nullptr;
        coctx_swap(&(co->m_coctx), &(t_main_coroutine->m_coctx));
    }

    // form main cor switch to target cor
    void Coroutine::Resume(Coroutine *co)
    {
        if (t_cur_coroutine != t_main_coroutine)
        {
            ErrorLog << "swap error! current cor must be mian cor";
            return;
        }

        if (t_main_coroutine == nullptr)
        {
            ErrorLog << "main cor is nullptr";
            return;
        }

        if (co == nullptr || co->m_can_resume == false)
        {
            ErrorLog << "pending cor is nullptr or can_resumr is false";
            return;
        }

        if (t_cur_coroutine == co)
        {
            DebugLog << "current cor is pending cor, don't need swap";
            return;
        }
        t_cur_coroutine = co;
        t_cur_run_time = co->getRunTime();
        coctx_swap(&(t_main_coroutine->m_coctx), &(co->m_coctx));
    }
}
