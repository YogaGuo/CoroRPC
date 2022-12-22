/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-20 16:01:41
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-22 15:54:29
 */
#ifndef COMPACTRPC_TCP_SERVER_H
#define COMPACTRPC_TCP_SERVER_H

#include <map>
#include <google/protobuf/service.h>
#include "compactrpc/net/reactor.h"
#include "compactrpc/net/fd_event.h"
#include "compactrpc/net/net_address.h"
#include "compactrpc/net/timer.h"
#include "compactrpc/net/tcp/tcp_conn_time_wheel.h"
#include "compactrpc/net/tcp/io_thread.h"
namespace compactrpc
{
    class TcpAcceptor
    {
    public:
        typedef std::shared_ptr<TcpAcceptor> ptr;

        TcpAcceptor(NetAddress::ptr net_addr);

        ~TcpAcceptor();

        void init();

        int toAccept();

        NetAddress::ptr getLocalAddr()
        {
            return m_local_addr;
        }

        NetAddress::ptr getPeerAddr()
        {
            return m_peer_addr;
        }

    private:
        int m_family{-1};
        int m_fd{-1};

        NetAddress::ptr m_local_addr{nullptr};
        NetAddress::ptr m_peer_addr{nullptr};
    };

    class TcpServer
    {
    public:
        typedef std::shared_ptr<TcpServer> ptr;

        TcpServer(NetAddress::ptr addr, ProtocalType type = TinyPb_Protocal);

        ~TcpServer();

        void start();

        void addCoroutine(compactrpc::Coroutine::ptr cor);

        bool regiterServer(std::shared_ptr<google::protobuf::Service> service);

        bool registerHttpServlet(const std::string &url_path, HttpServlet::ptr servlet);

        TcpConnection::ptr addClient(IOThread *io_thread, int fd);

        void freshTcpConnection(TcpTimeWheel::TcpConnectionSlot::ptr slot);

    public:
        // AbstractDispathcher::ptr getDispatcher();
        // AbstractCodec::ptr getCodec();

        NetAddress::ptr getPeerAddr();

        NetAddress::ptr getLocalAddr();

        NetAddress::ptr getIOThreadPool();

        TcpTimeWheel::ptr getTimeWheel();

    private:
        void MainAcceptCorFunc();

        void ClearClientTimerFunc();

        NetAddress::ptr m_addr;

        TcpAcceptor::ptr m_acceptor;

        int m_tcp_count{0};

        Reactor *m_main_reactor{nullptr};

        bool m_is_stop_accept{false};

        Coroutine::ptr m_accepet_cor;

        /*
         to do sth;
        */
        ProtocalType m_protocal_type{Typepb_Protocal};

        TcpTimeWheel::ptr m_time_wheel;

        std::map<int, std::shared_ptr<TcpConnection>> m_clients;

        TimerEvent::ptr m_clear_client_timer_event{nullptr};
    };
}
#endif