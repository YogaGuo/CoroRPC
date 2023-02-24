/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-20 16:01:41
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-24 17:28:31
 */
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "tcp_server.h"
#include "compactrpc/comm/config.h"
#include "compactrpc/coroutine/coroutine_pool.h"
#include "compactrpc/net/tinypb/tinypb_rpc_dispatcher.h"
namespace compactrpc
{
    extern compactrpc::Config::ptr gRpConfig;

    TcpAcceptor::TcpAcceptor(NetAddress::ptr net_addr) : m_local_addr(net_addr)
    {
        m_family = m_local_addr->getFamily();
    }

    void TcpAcceptor::init()
    {
        m_fd = socket(m_local_addr->getFamily(), SOCK_STREAM, 0);
        if (m_fd < 0)
        {
            ErrorLog << "start server error, socket error, sys error =" << strerror(errno);
            Exit(0);
        }
        DebugLog << "create listenfd succ, listenfd = " << m_fd;

        int val = 1;
        if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
        {
            ErrorLog << "set RESUSEADDR error";
        }
        socklen_t len = m_local_addr->getSockLen();
        int rt = bind(m_fd, m_local_addr->getSockAddr(), len);
        if (rt != 0)
        {
            ErrorLog << "start server error, bind error, errno = " << strerror(errno);
            Exit(0);
        }
        DebugLog << "set RESUSEADDR succ";
        rt = listen(m_fd, 10);
        if (rt != 0)
        {
            ErrorLog << "start server error, listen error, fd = " << m_fd << "errno = " << strerror(errno);
            Exit(0);
        }
    }
    TcpAcceptor::~TcpAcceptor()
    {
        FdEvent::ptr fd_event = FdEventContianer::GetFdContianer()->getFdEvent(m_fd);
        fd_event->unregisterFromReactor();
        if (m_fd != -1)
        {
            close(m_fd);
        }
    }

    int TcpAcceptor::toAccept()
    {
        socklen_t len = 0;
        int rt = 0;

        if (m_family == AF_INET)
        {
            sockaddr_in cli_addr;
            memset(&cli_addr, 0, sizeof(cli_addr));
            len = sizeof(cli_addr);

            DebugLog << "start call accept_hook";
            rt = accept_hook(m_fd, reinterpret_cast<sockaddr *>(&cli_addr), &len);
            if (rt == -1)
            {
                ErrorLog << "error, no new client coming, error=" << strerror(errno);
                return -1;
            }
            InfoLog << "New client accept succ! port: [ " << cli_addr.sin_port;
            m_peer_addr = std::make_shared<IPAddress>(cli_addr);
        }
        else if (m_family == AF_UNIX)
        {
            sockaddr_un cli_addr;
            len = sizeof(cli_addr);
            memset(&cli_addr, 0, sizeof(cli_addr));

            DebugLog << "start call accept_hook";
            rt = accept_hook(m_fd, reinterpret_cast<sockaddr *>(&cli_addr), &len);
            if (rt == -1)
            {
                ErrorLog << "error, no new client coming, error=" << strerror(errno);
                return -1;
            }
            InfoLog << "New client accept succ! port: [ " << cli_addr.sin_port;
            m_peer_addr = std::make_shared<UnixDomainAddress>(cli_addr);
        }
        else
        {
            ErrorLog << "unknown type protocal";
            close(m_fd);
            return -1;
        }
        InfoLog << "New client accepted succ, fd:[ " << rt << "addr: [ " << m_peer_addr->toString() << " ]";
    }

    TcpServer::TcpServer(NetAddress::ptr addr, ProtocalType type /*TinyPd_Protocal*/)
        : m_addr(addr)
    {
        m_io_pool = std::make_shared<IOThreadPool>(gRpcConfig->m_iothread_num);
        if (type == Http_Protocal)
        {
            // m_dispatcher = std::make_shared<HttpDis>
        }
        else
        {
            m_dispatcher = std::make_shared<TinyPbRpcDispacther>();
            m_codec = std::make_shared<TinyPbCodeC>();
            m_protocal_type = TinyPb_Protocal;
        }

        m_main_reactor = compactrpc::Reactor::GetReactor();
        m_main_reactor->setReactorType(MainReactor);
        m_time_wheel = std::make_shared<TcpTimeWheel>(m_main_reactor, gRpcConfig->m_timeWheel_bucket_num,
                                                      gRpcConfig->m_timeWheel_inteval);
        m_clear_client_timer_event = std::make_shared<TimerEvent>(1000, true,
                                                                  std::bind(TcpServer::ClearClientTimerFunc, this));

        m_main_reactor->getTimer()->addTimerEvent(m_clear_client_timer_event);

        InfoLog << "TcpServer setup on [ " << m_addr->toString() << " ]";
    }

    void TcpServer::start()
    {
        m_acceptor.reset(new TcpAcceptor(m_addr));
        m_acceptor->init();
        m_accepet_cor = GetCoroutinePool()->getCoroutineInstance();
        m_accepet_cor->setCallBack(std::bind(TcpServer::MainAcceptCorFunc, this));

        InfoLog << "resume accept corouine";
        compactrpc::Coroutine::Resume(m_accepet_cor.get());

        m_io_pool->start();
        m_main_reactor->loop();
    }

    TcpServer::~TcpServer()
    {
        GetCoroutinePool()->returnCoroutine(m_accepet_cor);
        DebugLog << "~TcpServer";
    }

    void TcpServer::MainAcceptCorFunc()
    {
        while (!m_is_stop_accept)
        {
            int fd = m_acceptor->toAccept();
            if (fd == -1)
            {
                ErrorLog << "accept ret -1 error, return, ti yield";
                Coroutine::Yield();
                continue;
            }

            IOThread *io_thread = m_io_pool->getIOThread();
            TcpConnection::ptr conn = addClient(io_thread, fd);
            conn->initServer();
            DebugLog << "tcpconnection address is " << conn.get() << ", and fd is" << fd;
            m_tcp_count++;
            DebugLog << "current tcp connection count is [ " << m_tcp_count << " ]";
        }
    }

    void TcpServer::addCoroutine(Coroutine::ptr cor)
    {
        m_main_reactor->addCoroutine(cor);
    }

    bool TcpServer::regiterServer(std::shared_ptr<google::protobuf::Service> service)
    {
        if (m_protocal_type == TinyPb_Protocal)
        {
            if (service)
            {
                dynamic_cast<TinyPbRpcDispacther *>(m_dispatcher.get())->registerService(service);
            }
            else
            {
                ErrorLog << "register service error, service ptr is nullptr";
                return false;
            }
        }
        else
        {
            ErrorLog << "register service error, Just TinyPb protocal server need to register Service";
            return false;
        }
        return true;
    }

    TcpConnection::ptr TcpServer::addClient(IOThread *io_thread, int fd)
    {
        auto it = m_clients.find(fd);
        if (it != m_clients.end())
        {
            it->second.reset();
            DebugLog << "fd " << fd << "have already exit, reset it";
            it->second = std::make_shared<TcpConnection>(this, io_thread, fd, 128, getPeerAddr());
            return it->second;
        }
        else
        {
            DebugLog << "fd " << fd << "did't exit , need create it";
            TcpConnection::ptr conn = std::make_shared<TcpConnection>(this, io_thread, fd, 128, getPeerAddr());
            m_clients.insert({fd, conn});
            return conn;
        }
    }

    void TcpServer::freshTcpConnection(TcpTimeWheel::TcpConnectionSlot::ptr slot)
    {
        auto cb = [slot, this]()
        {
            this->getTimeWheel()->fresh(slot);
        };
        m_main_reactor->addTask(cb);
    }

    /**
     * @brief : this IOThread loop's timer exec;
     *
     */
    void TcpServer::ClearClientTimerFunc()
    {
        for (auto &ret : m_clients)
        {
            if (ret.second && ret.second.use_count() > 0 && ret.second->getState() == Closed)
            {
                DebugLog << "TcpConnection [ fd" << ret.first << " ] will delele ,state = " << ret.second->getState();
                ret.second.reset();
            }
        }
    }

    NetAddress::ptr TcpServer::getPeerAddr()
    {
        return m_acceptor->getPeerAddr();
    }

    NetAddress::ptr TcpServer::getLocalAddr()
    {
        return m_acceptor->getLocalAddr();
    }

    TcpTimeWheel::ptr TcpServer::getTimeWheel()
    {
        return m_time_wheel;
    }

    IOThreadPool::ptr TcpServer::getIOThreadPool()
    {
        return m_io_pool;
    }

}