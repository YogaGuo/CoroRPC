/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-02-21 11:06:57
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-24 19:47:12
 */
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "compactrpc/comm/error_code.h"
#include "compactrpc/net/tcp/tcp_client.h"
#include "compactrpc/net/tinypb/tinypb_rpc_channel.h"
#include "compactrpc/net/tinypb/tinypb_rpc_controller.h"
#include "compactrpc/net/tinypb/tinypb_codec.h"
#include "compactrpc/net/tinypb/tinypb_data.h"
#include "compactrpc/comm/log.h"
#include "compactrpc/comm/run_time.h"
#include "compactrpc/comm/msg_req.h"

namespace compactrpc
{

    TinyPbRpcChannel::TinyPbRpcChannel(NetAddress::ptr addr) : m_addr(addr) {}

    /**
     * @brief :  TinyPbRpcChannel继承自(protobuf中的RpcChannel),主要任务：
     *               把请求数据包装， 然后序列化得到二进制， 在发送出去
     * @param method
     * @param controller
     * @param request
     * @param response
     * @param done
     */
    void TinyPbRpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                                      google::protobuf::RpcController *controller,
                                      const google::protobuf::Message *request,
                                      google::protobuf::Message *response,
                                      google::protobuf::Closure *done)
    {
        TinyPbStruct pb_struct;
        TinyPbRpcController *rpc_controller = dynamic_cast<TinyPbRpcController *>(controller);
        if (!rpc_controller)
        {
            ErrorLog << "call faild. falid to dynamic_cast TinyPbRpcController";
            return;
        }
        TcpClient::ptr m_client = std::make_shared<TcpClient>(m_addr);
        rpc_controller->SetLocalAddr(m_client->getLocalAddr());
        rpc_controller->SetPeerAddr(m_client->getPeerAddr());

        pb_struct.service_full_name = method->full_name();
        DebugLog << "call service_name = " << pb_struct.service_full_name;

        if (!request->SerializeToString(&pb_struct.pb_data))
        {
            ErrorLog << "serialize send package error";
            return;
        }

        if (!rpc_controller->MsgSeq().empty())
        {
            pb_struct.msg_req = rpc_controller->MsgSeq();
        }
        else
        {
            // get current coroutine"s msgno to set this request
            RunTime *runtime = getCurrentRunTime();
            if (runtime != nullptr && runtime->m_msg_no.empty())
            {
                pb_struct.msg_req = runtime->m_msg_no;
                DebugLog << "get from RunTime succ, msgno = " << pb_struct.msg_req;
            }
            else
            {
                pb_struct.msg_req = MsgReqUtil::getMsgNumber();
                DebugLog << "get from RunTime error, generator new msgno = " << pb_struct.msg_req;
            }
            rpc_controller->SetMsgReq(pb_struct.msg_req);
        }

        AbstractCodeC::ptr m_codec = m_client->getConnection()->getCodeC();
        m_codec->encode(m_client->getConnection()->getOutBuffer(), &pb_struct);
        if (!pb_struct.encode_succ)
        {
            rpc_controller->SetError(ERROR_FAILED_ENCODE, "encode tinypb data error");
            return;
        }

        InfoLog << "==================================================";
        InfoLog << pb_struct.msg_req << "|" << rpc_controller->PeerAddr()->toString()
                << "|. Set client send request data: " << request->ShortDebugString();
        InfoLog << "==================================================";

        m_client->setTimeout(rpc_controller->Timeout());

        TinyPbStruct::pb_ptr res_data;
        int rt = m_client->sendAndRecvTinyPb(pb_struct.msg_req, res_data);
        if (rt != 0)
        {
            rpc_controller->SetError(rt, m_client->getErrInfo());
            ErrorLog << pb_struct.msg_req << "| call rpc occur client error, ervice_full_name = " << pb_struct.service_full_name;
            return;
        }
        if (!response->ParseFromString(res_data->pb_data))
        {
            rpc_controller->SetError(ERROR_FAILED_DESERIALIZE, "faild to descri data from server");
            ErrorLog << pb_struct.msg_req << "| failed to deseria data";
            return;
        }

        if (res_data->err_code != 0)
        {
            ErrorLog << pb_struct.msg_req << "| server reply error_code = " << res_data->err_code return;
        }

        InfoLog << "=============================================";
        InfoLog << pb_struct.msg_req << "| " << rpc_controller->PeerAddr()->toString()
                << "| call rpc server [ " << pb_struct.service_full_name << " ] succ"
                << ".  Get server reply response data:" << response->ShortDebugString();
        InfoLog << "=============================================";

        if (done)
        {
            done->Run();
        }
    }
}