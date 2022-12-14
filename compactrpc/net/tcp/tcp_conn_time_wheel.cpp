/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-14 14:14:26
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-14 14:44:36
 */
#include "compactrpc/net/tcp/tcp_conn_time_wheel.h"

namespace compactrpc
{
    TcpTimeWheel::TcpTimeWheel(Reactor *reactor, int bucket_count, int inteval) : m_reactor(reactor), m_bucket_count(bucket_count), m_inteval(inteval)
    {
        for (int i = 0; i < m_bucket_count; i++)
        {
            std::vector<TcpConnectionSlot::ptr> tmp;
            m_wheel.push(tmp);
        }

        m_event = std::make_shared<TimeEvent>(m_inteval * 1000, true,
                                              std::bind(&TcpTimeWheel::loopFunc(), this));
        m_reactor->getTimer()->addTimeEvent(m_event);
    }

    TcpTimeWheel::~TcpTimeWheel()
    {
        m_reactor->getTimer()->delTimeEvent();
    }

    void TcpTimeWheel::loopFunc()
    {
        m_wheel.pop();
        std::vector<TcpConnectionSlot::ptr> tmp;
        m_wheel.push(tmp);
    }

    void TcpTimeWheel::fresh(TcpConnectionSlot::ptr slot)
    {
        DebugLog << "fresh connection";
        m_wheel.back().emplace_back(slot);
    }
}