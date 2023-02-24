/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-02-19 16:03:22
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-20 20:41:26
 */
#ifndef COMPACTRPC_NET_TINYPB_TINYPB_RPC_CONTROLLER_H
#define COMPACTRPC_NET_TINYPB_TINYPB_RPC_CONTROLLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <stdio.h>
#include <memory>
#include "compactrpc/net/net_address.h"

namespace compactrpc
{
    class TinyPbRpcController : public google::protobuf::RpcController
    {
    public:
        typedef std::shared_ptr<TinyPbRpcController> ptr;

        /**
         * @brief : Client method;
         *
         */
        TinyPbRpcController() = default;
        ~TinyPbRpcController() = default;

        void Reset() override;
        bool Failed() const override;

        /**
         * @brief : Server method
         *
         */
        std::string ErrorText() const override;

        void StartCancel() override;

        void SetFailed(const std::string &reason) override;

        bool IsCanceled() const override;

        void NotifyOnCancel(google::protobuf::Closure *callback) override;

        /**
         * @brief : common method
         *
         */
        int ErrorCode() const;
        void SetErrorCode(const int error_code);
        const std::string &MsgSeq() const;
        void SetMsgReq(const std::string &msg_req);
        void SetError(const int err_code, const std::string &err_info);
        void SetPeerAddr(NetAddress::ptr addr);
        void SetLocalAddr(NetAddress::ptr addr);
        NetAddress::ptr PeerAddr();
        NetAddress::ptr LocalAddr();

        void SetTimeout(const int timeout);
        int Timeout() const;

        void SetMethodName(const std::string &name);

        std::string GetMethodName();

        void SetMethodName(const std::string &name);

        std::string GetMethodFullName();

        void SetMethodFullName(const std::string &name);

        std::string GetMethodFullName();

    private:
        int m_error_code{0};      // error_code, indetify one specific error;
        std::string m_error_info; // datials description of error;
        std::string m_msg_req;    // identify once rpc request and response

        bool m_is_failed{false};
        bool m_is_cancel{false};

        NetAddress::ptr m_peer_addr;
        NetAddress::ptr m_local_addr;

        int m_timeout{5000}; // max call rpc_timeout;
        std::string m_method_name;
        std::string m_full_name; // like: server.method_name;
    };
}
#endif