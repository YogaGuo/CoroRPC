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
          ����service_full_name, �õ� service_name method_name
          ����ʧ��
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
             * @brief ������Ӧ���ݰ��� ���͸� TcpConnection����
             *
             */
            conn->getCodeC()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData *>(&reply_pk));
            return;
        }
        Coroutine::GetCurrentCoroutine()->getRunTime()->m_interface_name = tmp->service_full_name;

        /**
         * @brief �ҵ���Ӧ��service
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
         * @brief: �ҵ���Ӧ method
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
         * @brief : ���� method ������� request �� response ����
         */
        google::protobuf::Message *request = service->GetRequestPrototype(method).New();
        DebugLog << reply_pk.msg_req << "| request.name = " << request->GetDescriptor()->full_name();

        /**
         * @brief: ���ͻ������ݰ������ protobuf �ֽ��������л�Ϊ request����
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

        // ��ʼ��response ����
        google::protobuf::Message *response = service->GetResponsePrototype(method).New();

        DebugLog << reply_pk.msg_req << "| response.name = " << response->GetDescriptor()->full_name();

        TinyPbRpcController rpc_controller;
        rpc_controller.SetMsgReq(reply_pk.msg_req);
        rpc_controller.SetMethodName(method_name);
        rpc_controller.SetMethodFullName(tmp->service_full_name);

        std::function<void()> reply_package_func = []() {};

                // ����ҵ������
        service->CallMethod(method, &rpc_controller, request, response)
    }
}