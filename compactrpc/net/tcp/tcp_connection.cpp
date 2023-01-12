/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-03 19:03:13
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-12 19:08:05
 */
#include "compactrpc/net/tcp/tcp_connection.h"
#include <unistd.h>
#include <string.h>
// #include "compactrpc/net/...."
// #include "compactrpc/net/..."
#include "compactrpc/coroutine/coroutine_pool.h"
#include "compactrpc/coroutine/coroutine_hook.h"
#include "compactrpc/net/timer.h"
#include "compactrpc/net/tcp/tcp_conn_time_wheel.h"

namespace compactrpc
{
    TcpConnection::TcpConnection(compactrpc::TcpServer *tcp_ser, compactrpc::IOThread *io_thread, int fd, int buff_size, NetAddress::ptr peer_addr)
        : m_io_thread(io_thread), m_fd(fd), m_state(Connected), m_connection_type(ServerConnection), m_peer_addr(peer_addr)
    {
        m_reactor = m_io_thread->getRector();

        m_tcp_server = tcp_ser;

        m_codeC = m_tcp_server->getCodeC();
        m_fd_event = FdEventContianer::GetFdContianer()->getFdEvent(fd);
        m_fd_event->setReactor(m_reactor);
        initBuffer(buff_size);
        m_loop_cor = getCoroutinePool()->getCoroutineInstance();
        m_state = Connected;
        DebugLog << "succ create tcp_connection[ " << m_state << " ], fd = " << fd;
    }

    TcpConnection::TcpConnection(compactrpc::TcpClient *tcp_cli, compactrpc::Reactor *reactor, int fd, int buff_size, NetAddress::ptr peer_addr)
        : m_fd(fd), m_state(NotConnected), m_connection_type(ClientConnection), m_peer_addr(peer_addr)
    {
        m_reactor = reactor;

        m_tcp_cli = tcp_cli;

        m_codec = m_tcp_cli->getCodeC();

        m_fd_event = FdEventContianer::GetFdContianer()->getFdEvent(fd);
        m_fd_event->setReactor(m_reactor);
        initBuffer(buff_size);

        DebugLog << "succ create tcp connection [NotConnectioned]";
    }

    void TcpConnection::initServer()
    {
        m_loop_cor->setCallBack(std::bind(TcpConnection::MainServerLoopCorFunc, this));
    }

    void TcpConnection::registerToTimeWheel()
    {
        auto cb = [](TcpConnection::ptr conn)
        {
            conn->shutdownConnection();
        };
        TcpTimeWheel::TcpConnectionSlot::ptr tmp = std::make_shared<AbstractSlot<TcpConnection>>(shared_from_this(), cb);
        m_weak_slot = tmp;

        m_tcp_server->freshTcpConnection(tmp);
    }

    void TcpConnection::setUpClient()
    {
        setState(Connected);
    }

    TcpConnection::~TcpConnection()
    {
        if (m_connection_type == ServerConnection)
        {
            getCoroutinePool()->returnCoroutine(m_loop_cor);
        }
        DebugLog << "~TcpConnection, fd = " << m_fd;
    }

    void TcpConnection::initBuffer(int size)
    {
        m_write_buffer = std::make_shared<TcpBuff>(size);
        m_read_buffer = std::make_shared<TcpBuff>(size);
    }

    void TcpConnection::MainServerLoopCorFunc()
    {
        while (!m_stop)
        {
            input();

            execute();

            output();
        }
        InfoLog << "this connection has already end loop";
    }

    void TcpConnection::input()
    {
        if (m_is_over_time)
        {
            InfoLog << "over timer, skip input progress";
            return;
        }

        TcpConnectionState state = getState();
        if (state == Closed || state == NotConnected)
        {
            return;
        }
        bool read_all = false;
        bool closed_flag = false;
        int count = 0;
        while (!read_all)
        {
            if (m_read_buffer->writeAble() == 0)
            {
                m_read_buffer->resizeBuffer(2 * m_read_buffer->getSize());
            }

            int read_count = m_read_buffer->writeAble();
            int write_index = m_read_buffer->writeIndex();

            DebugLog << "m_read_buffer size = " << m_read_buffer->getBufferVector().size() << "rd = " << m_read_buffer->readIndex();

            int rt = read_hook(m_fd, &(m_read_buffer->m_buffer[write_index]), read_count);
            if (rt > 0)
            {
                m_read_buffer->recycleWrite(rt);
            }

            DebugLog << "m_read_buffer size = " << m_read_buffer->getBufferVector().size()
                     << "rd = " << m_read_buffer->readIndex() << "wd = " << m_read_buffer->writeIndex();

            DebugLog << "read data back, fd = " << m_fd;
            count += rt;

            if (m_is_over_time)
            {
                InfoLog << "over timer, now break read function";
                break;
            }
            if (rt <= 0)
            {
                DebugLog << "rt <= 0";
                ErrorLog << "read empty while occur read event, because of peer close, fd = " << m_fd << ", sys error = "
                         << strerror(errno) << ", now to clear tcp connection";
                closed_flag = true;
                break;
            }
            else
            {
                if (rt == read_count)
                {
                    DebugLog << "read_count == rt";
                    continue;
                }
                else if (rt < read_count)
                {
                    DebugLog << "read_count > rt";
                    read_all = true;
                    break;
                }
            }
        }
        if (closed_flag)
        {
            clearClient();
            DebugLog << "peer close, now yield current coroutine, wait main thread clear this TcpConnection";
            Coroutine::GetCurrentCoroutine()->setCanResume(false);
            Coroutine::Yield();
        }

        if (m_is_over_time)
            return;
        if (!read_all)
        {
            ErrorLog << "not read all data in socket buffer";
        }
        InfoLog << "recv [ " << count << " ] bytes data from [ " << m_peer_addr->toString() << " ]" << m_fd << " ]";
        if (m_connection_type == ServerConnection)
        {
            TcpTimeWheel::TcpConnectionSlot::ptr tmp = m_weak_slot.lock();
            if (tmp)
            {
                m_tcp_server->freshTcpConnection(tmp);
            }
        }
    }

    void TcpConnection::execute()
    {
    }

    void TcpConnection::output()
    {
        if (m_is_over_time)
        {
            InfoLog << " over timer, skip output progress";
            return;
        }

        while (true)
        {
            TcpConnectionState state = getState();
            if (state != Connected)
            {
                break;
            }
            if (m_write_buffer->readAble() == 0)
            {
                DebugLog << "app buffer of fd [ " << m_fd << " ] no data to write, to yiled this coroutine";
                break;
            }
            int total_size = m_write_buffer->readAble();
            int read_index = m_write_buffer->readIndex();
            int rt = write_hook(m_fd, &(m_write_buffer->m_buffer[read_index]), total_size);

            if (rt <= 0)
            {
                ErrorLog << "write empty, error = " << strerror(errno);
            }
            DebugLog << "succ write " << rt << "bytes";
            m_write_buffer->recycleRead(rt);
            DebugLog << "recycle write index = " << m_write_buffer->writeIndex() << ", read_index =" << m_write_buffer->readIndex() << "readable = " << m_write_buffer->readAble();
            IofoLog << "send [ " << rt << " ] bytes data to [ " << m_peer_addr->toString() << " ],fd [ "
                    << m_fd << " ]";

            if (m_write_buffer->readAble() == 0)
            {
                InfoLog << "send all data, now unregister write_event and break" break;
            }

            if (m_is_over_time)
            {
                InfoLog << "over timer, now break write function";
                break;
            }
        }
    }

    void TcpConnection::clearClient()
    {
        if (getState() == Closed)
        {
            DebugLog << "this client has closed";
            return;
        }

        // first unregister  form epoll
        m_fd_event->unregisterFromReactor();

        // stop read & write's cor
        m_stop = true;

        close(m_fd_event->getFd());
        setState(Closed);
    }

    void TcpConnection::shutdownConnection()
    {
        TcpConnectionState state = getState();
        if (state == Closed || state == NotConnected)
        {
            DebugLog << "this client has closed";
            return;
        }
        setState(HalfClosing);
        InfoLog << "shutdown conn [ " << m_peer_addr->toString() << " ], fd = " << m_fd;

        /**
         * @brief call sys shutdown to send FIN, then, fd ocuur read_event,but read's
         *        bytes == 0, then will call clearClient() to close()
         */
        shutdown(m_fd_event->getFd(), SHUT_RDWR);
    }

    TcpBuffer *TcpConnection::getInBuffer()
    {
        return m_read_buffer.get();
    }

    TcpBuffer *TcpConnection::getOutBuffer()
    {
        return m_write_buffer.get();
    }

    bool TcpConnection::getResPackageData(const std::string &msg_req, TinyPbStruct::pb_ptr &pb_strcut)
    {
    }

    AbstractCodeC::ptr TcpConnection::getCodeC() const
    {
    }

    TcpConnectionState TcpConnection::getState()
    {
        TcpConnectionState state;
        RWMutex::ReadLock lock(m_mutex);
        state = m_state;
        lock.unlock();

        return state;
    }

    void TcpConnection::setState(const TcpConnectionState &state)
    {
        RWMutex::WriteLock lock(m_mutex);
        m_state = state;
        lock.unlock();
    }

    void TcpConnection::setOverTimeFlag(bool v)
    {
        m_is_over_time = v;
    }

    bool TcpConnection::getOverTimeFlag()
    {
        return m_is_over_time;
    }

    Coroutine::ptr TcpConnection::getCoroutine()
    {
        return m_loop_cor;
    }
}