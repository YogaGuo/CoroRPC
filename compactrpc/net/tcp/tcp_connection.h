/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-03 18:57:35
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-24 16:58:09
 */
#ifndef COMPACTRPC_TCP_CONNECTION_H
#define COMPACTRPC_TCP_CONNECTION_H

#include <string>
#include <map>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "compactrpc/comm/log.h"
#include "compactrpc/net/fd_event.h"
#include "compactrpc/net/timer.h"
#include "compactrpc/net/tinypb/tinypb_codec.h"
#include "compactrpc/net/abstract_codec.h"
#include "compactrpc/net/tcp/tcp_client.h"
#include "compactrpc/net/tcp/tcp_server.h"
#include "compactrpc/net/tcp/io_thread.h"
#include "compactrpc/net/tcp/tcp_buffer.h"
#include "compactrpc/comm/config.h"
namespace compactrpc
{
    class TcpServer;
    class TcpClient;
    class IOThread;
    enum TcpConnectionState
    {
        NotConnected = 1, // can do io
        Connected = 2,    // can do io
        HalfClosing = 3,  // server call shutdown, write half close. can read but can't write
        Closed = 4,       // can't do io
    };

    class TcpConnection : public std::enable_shared_from_this<TcpConnection>
    {
    public:
        typedef std::shared_ptr<TcpConnection> ptr;
        TcpConnection(compactrpc::TcpServer *tcp_server, compactrpc::IOThread *io_thread, int fd, int buff_size, NetAddress::ptr peer_addr);

        TcpConnection(compactrpc::TcpClient *tcp_client, compactrpc::Reactor *reactor, int fd, int buff_size, NetAddress::ptr peer_addr);

        void setUpServer();

        void setUpClient();

        ~TcpConnection();

        void initBuffer(int size);

    public:
        enum ConnectionType
        {
            ServerConnection = 1, // owned by tcp_server;
            ClientConnection = 2, // owned by tcp_client
        };

        void shutdownConnection();

        TcpConnectionState getState();

        void setState(const TcpConnectionState &state);

        TcpBuffer *getInBuffer();

        TcpBuffer *getOutBuffer();

        AbstractCodeC::ptr getCodeC() const;

        bool getResPackageData(const std::string &msg_req, TinyPbStruct::pb_ptr &pb_strcut);

        void registerToTimeWheel();

        Coroutine::ptr getCoroutine();

    public:
        void MainServerLoopCorFunc();

        void input();

        void execute();

        void output();

        void setOverTimeFlag(bool v);

        bool getOverTimeFlag();

        void initServer();

    private:
        void clearClient();

        TcpServer *m_tcp_server{nullptr};
        TcpClient *m_tcp_cli{nullptr};
        IOThread *m_io_thread{nullptr};
        Reactor *m_reactor{nullptr};

        int m_fd{-1};
        TcpConnectionState m_state{TcpConnectionState::Connected};

        ConnectionType m_connection_type{ServerConnection};

        NetAddress::ptr m_peer_addr;

        TcpBuffer::ptr m_read_buffer;
        TcpBuffer::ptr m_write_buffer;

        Coroutine::ptr m_loop_cor;

        AbstractCodeC::ptr m_codec;

        FdEvent::ptr m_fd_event;

        bool m_stop{false};

        bool m_is_over_time{false};

        std::map<std::string, std::shared_ptr<TinyPbStruct>> m_reply_datas;

        std::weak_ptr<AbstractSlot<TcpConnection>> m_weak_slot;

        RWMutex m_mutex;
    };
}
#endif