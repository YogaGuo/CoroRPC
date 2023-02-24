/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-02-19 15:39:51
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-20 20:47:07
 */
#include "compactrpc/net/tinypb/tinypb_rpc_dispatcher.h"
#include "compactrpc/net/tinypb/tinypb_data.h"
#include "compactrpc/net/tinypb/tinypb_rpc_closure.h"
#include "compactrpc/net/tinypb/tinypb_rpc_controller.h"
#include "compactrpc/net/tinypb/tinypb_codec.h"
#include "compactrpc/comm/msg_req.h"
#include "compactrpc/comm/error_code.h"
#include <sstream>
namespace compactrpc
{
    class TcpBuffer;

    void TinyPbRpcDispacther::dispatch(AbstractData *data, TcpConnection *conn)
    {
        TinyPbStruct *tmp = dynamic_cast<TinyPbStruct *>(data);
        if (tmp == nullptr)
        {
            ErrorLog << "dynamic_cast error";
            return;
        }
        Coroutine::GetCurrentCoroutine()->getRunTime()->m_msg_no = tmp->msg_req;
        setCurrentRuntime(Coroutine::GetCurrentCoroutine()->getRunTime());

        InfoLog << "begin to dispatch client tinypb request, msgno = " << tmp->msg_req;

        std::string service_name;
        std::string method_name;

        TinyPbStruct reply_pk;
        reply_pk.service_full_name = tmp->service_full_name;
        reply_pk.msg_req = tmp->msg_req;
        if (reply_pk.msg_req.empty())
        {
            reply_pk.msg_req = MsgReqUtil::getMsgNumber();
        }

        /*
          解析service_full_name, 得到 service_name method_name
          解析失败
        */
        if (!parseServiceFullName(tmp->service_full_name, service_name, method_name))
        {
            ErrorLog << "reply_pk.msg_req"
                     << "parse service_name " << tmp->service_full_name << "error";
            reply_pk.err_code = ERROR_PARSE_SERVICE_NAME;
            std::stringstream ss;
            ss << "can not parse service_name : [ " << tmp->service_full_name << "]";
            reply_pk.err_info = ss.str();

            /**
             * @brief 编码响应数据包， 回送给 TcpConnection对象
             *
             */
            conn->getCodeC()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData *>(&reply_pk));
            return;
        }
        Coroutine::GetCurrentCoroutine()->getRunTime()->m_interface_name = tmp->service_full_name;

        /**
         * @brief 找到对应的service
         */
        auto it = m_service_map.find(service_name);
        if (it == m_service_map.end() || !it->second)
        {
            reply_pk.err_code = ERROR_SERVICE_NOT_FOUND;
            std::stringstream ss;
            ss << "not found service_name :[ " << service_name << " ]";
            ErrorLog << reply_pk.msg_req << "| " << ss.str();
            reply_pk.err_info = ss.str();
            conn->getCodeC()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData *>(&reply_pk));

            InfoLog << "end dispatch client tinypb request, msgno = " << tmp->msg_req;
            return;
        }
        service_ptr service = (*it).second;

        /**
         * @brief: 找到对应 method
         */
        const google::protobuf::MethodDescriptor *method = service->GetDescriptor()->FindMethodByName(method_name);

        if (!method)
        {
            reply_pk.err_code = ERROR_METHOD_NOT_FOUND;
            std::stringstream ss;
            ss << "not found method_name : [ " << method_name << " ]";
            ErrorLog << reply_pk.msg_req << "| " << ss.str();
            reply_pk.err_info = ss.str();
            conn->getCodeC()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData *>(&reply_pk));
            return;
        }

        /**
         * @brief : 根据 method 对象反射出 request 和 response 对象
         */
        google::protobuf::Message *request = service->GetRequestPrototype(method).New();
        DebugLog << reply_pk.msg_req << "| request.name = " << request->GetDescriptor()->full_name();

        /**
         * @brief: 将客户端数据包里面的 protobuf 字节流反序列化为 request对象
         */
        if (!request->ParseFromString(tmp->pb_data))
        {
            reply_pk.err_code = ERROR_FAILED_SERIALIZE;
            std::stringstream ss;
            ss << "faild to parse request data, request.name: [" << request->GetDescriptor()->full_name() << " ]";
            reply_pk.err_info = ss.str();
            ErrorLog << reply_pk.msg_req << "| " << ss.str();
            delete request;
            conn->getCodeC()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData *>(&reply_pk));
            return;
        }

        InfoLog << "===================================================";
        InfoLog << reply_pk.msg_req << "| Get client request data : " << request->ShortDebugString();
        InfoLog << "====================================================";

        // 初始化response 对象
        google::protobuf::Message *response = service->GetResponsePrototype(method).New();

        DebugLog << reply_pk.msg_req << "| response.name = " << response->GetDescriptor()->full_name();

        TinyPbRpcController rpc_controller;
        rpc_controller.SetMsgReq(reply_pk.msg_req);
        rpc_controller.SetMethodName(method_name);
        rpc_controller.SetMethodFullName(tmp->service_full_name);

        std::function<void()> reply_package_func = []() {};

                // 调用业务处理方法
        service->CallMethod(method, &rpc_controller, request, response)
    }
}