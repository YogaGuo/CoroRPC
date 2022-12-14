/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-09 21:14:49
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-14 12:03:51
 */
#ifndef COMPACTRPC_COROUTINE_H
#define COMPACTRPC_COROUTINE_H

#include <memory>
#include <functional>
#include <string>
#include "compactrpc/coroutine/coctx.h"
#include "compactrpc/comm/run_time.h"

namespace compactrpc
{
    int getCoroutineIndex();

    Runtime *getCurrentRuntime();

    void setCurrentRuntime(RunTime *v);

    class Coroutine
    {
    public:
        typedef std::shared_ptr<Coroutine> ptr;

        Coroutine(int size, char *stack_ptr);

        Coroutine(int size, char *stack_ptr, std::function<void()> cb);

        ~Coroutine();

        bool setCallBack(std::function<void()>);

        int getCorid() const
        {
            return m_cor_id;
        }

        void setIsInCorFunc(const bool v)
        {
            m_is_in_cofunc = v;
        }

        bool getIsInCorFunc() const
        {
            return m_is_in_cofunc;
        }

        std::string getMsg()
        {
            return m_msg_no;
        }

        RunTime *getRunTime()
        {
            return &m_run_time;
        }

        void setMsgNo(const std::string &msg_no)
        {
            m_msg_no = msg_no;
        }

        void setIndex(int index)
        {
            m_index = index;
        }

        int getIndex()
        {
            return m_index;
        }

        char *getStackPtr()
        {
            return m_stack_sp;
        }

        void setCanResume(bool v)
        {
            m_can_resume = v;
        }

    public:
        static void Yield();

        static void Resume(Coroutine *co);

        static Coroutine *GetCurrentCoroutine();

        static Coroutine *GetMainCoroutine();

        static bool IsMainCoroutine();

        std::function<void()> m_call_back;

    private:
        Coroutine();

        int m_cor_id{0};            // cor id
        coctx m_coctx;              // cor regs[]
        int m_stack_size{0};        // size of stack
        char *m_stack_sp{nullptr};  // cor's mem space
        bool m_is_in_cofunc{false}; // when call Cofunction is true, otherwise, false;
        std::string m_msg_no;
        RunTime m_run_time;

        bool m_can_resume{false};
        int m_index{-1}; // index in cor pool
    };
}
#endif