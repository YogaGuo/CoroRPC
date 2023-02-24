/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-14 14:14:26
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-22 11:37:42
 */
#include <queue>
#include <vector>
#include "compactrpc/net/tcp/abstract_slot.h"
#include "compactrpc/net/reactor.h"
#include "compactrpc/net/timer.h"
#include "compactrpc/comm/log.h"
namespace compactrpc
{
    class TcpConnection;

    class TcpTimeWheel
    {
    public:
        /**
         * @brief ??Tcpconnction ????AbstractSlot,
         *        TcpConnectionSlot::ptr == std::shared<AbstractSlot>
         *        ??? ?????????????std::shared<AbstractSlot> ????Tcpconne??
         *
         */
        typedef std::shared_ptr<TcpTimeWheel> ptr;
        typedef AbstractSlot<TcpConnection> TcpConnectionSlot;

        TcpTimeWheel(Reactor *reactor, int bucket_count, int intetal = 10);

        ~TcpTimeWheel();

        void fresh(TcpConnectionSlot::ptr slot);

        void loopFunc();

    private:
        Reactor *m_reactor{nullptr};
        int m_bucket_count{0};
        int m_inteval{0}; // ?????????????? ??????????

        TimerEvent::ptr m_event;

        std::queue<std::vector<TcpConnectionSlot::ptr>> m_wheel;
    };
}