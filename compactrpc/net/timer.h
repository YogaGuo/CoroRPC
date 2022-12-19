/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-12 10:15:59
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-15 13:13:40
 */
#ifndef COMPACTRPC_TIMER_H
#define COMPACTRPC_TIMER_H

#include <time.h>
#include <memory>
#include <map>
#include <functional>
#include "compactrpc/comm/Mutex.h"
#include "compactrpc/net/reactor.h"
#include "compactrpc/net/fd_event.h"
#include "compactrpc/comm/log.h"
namespace compactrpc
{
    int64_t getNowMs();

    class TimerEvent
    {
    public:
        typedef std::shared_ptr<TimerEvent> ptr;
        TimerEvent(int64_t interval, bool is_repeated, std::function<void()> task)
            : m_interval(interval), m_is_repeated(is_repeated), m_task(task)
        {
            m_arrive_time = getNowMs() + m_interval;
            DebugLog << "timeevent will occur at:" << m_arrive_time;
        }

        void resetTime()
        {
            m_arrive_time = getNowMs() + m_interval;
            m_is_cancled = false;
        }

        void wake()
        {
            m_is_cancled = false;
        }

        void cancle()
        {
            m_is_cancled = true;
        }

        void cancleRepeated()
        {
            m_is_repeated = false;
        }

    public:
        int64_t m_arrive_time; // when to exec task, (ms)
        int64_t m_interval;    // interval btw two task
        bool m_is_repeated{false};
        bool m_is_cancled{false};

        std::function<void()> m_task;
    };

    class FdEvent;

    class Timer : public compactrpc::FdEvent
    {
    public:
        typedef std::shared_ptr<Timer> ptr;

        Timer(Reactor *reactor);

        ~Timer();

        void addTimerEvent(TimerEvent::ptr event, bool need_reset = true);

        void delTimerEvent(TimerEvent::ptr event);

        void resetArriveTime();

        void onTimer();

    private:
        std::multimap<int64_t, TimerEvent::ptr> m_pendging_events;
    };
}
#endif