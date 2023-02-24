/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-03 18:57:35
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-22 17:47:06
 */
/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-03 18:57:35
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-21 18:45:11
 */
#ifndef COMPACTRPC_TCP_CLIENT
#define COMPACTRPC_TCP_CLIENT
#include <memory>
#include <string>
#include <google/protobuf/service.h>
#include "compactrpc/coroutine/coroutine_hook.h"
#include "compactrpc/coroutine/coroutine.h"
#include "compactrpc/net/net_address.h"
#include "compactrpc/net/reactor.h"
#include "compactrpc/net/tcp/tcp_connection.h"
#include "compactrpc/net/tinypb/tinypb_codec.h"
#include "compactrpc/net/http/http_codec.h"
namespace compactrpc
{

    class TcpClient
    {
    public:
        typedef std::shared_ptr<TcpClient> ptr;
        TcpClient(NetAddress::ptr addr, ProtocalType type = TinyPb_Protocal);

        ~TcpClient();

        void init();

        void resetFd();

        int sendAndRecvTinyPb(const std::string &msg_no, TinyPbStruct::pb_ptr &res);

        void stop();

        TcpConnection *getConnection();

        void setTimeout(const int v)
        {
            m_max_timeout = v;
        }

        void setTryCounts(const int v)
        {
            m_try_counts = v;
        }

        const std::string &getErrInfo()
        {
            return m_err_info;
        }

        NetAddress::ptr getPeerAddr() const
        {
            return m_peer_addr;
        }

        NetAddress::ptr getLocalAddr() const
        {
            return m_local_addr;
        }

        AbstractCodeC::ptr getCodeC()
        {
            return m_codeC;
        }

    private:
        int m_family{0};
        int m_fd{-1};
        int m_try_counts{3};      // max try reconnect times
        int m_max_timeout{10000}; // max connect timeout ,ms
        bool m_is_stop{false};
        std::string m_err_info; // error info client

        NetAddress::ptr m_local_addr{nullptr};
        NetAddress::ptr m_peer_addr{nullptr};
        Reactor *m_reactor{nullptr};

        TcpConnection::ptr m_connection{nullptr};

        AbstractCodeC::ptr m_codeC{nullptr};

        bool m_connect_succ{false};
    };
}
#endif
