/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-12 21:16:40
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-12 22:10:15
 */
#ifndef COMPACTRPC_TINYPB_DATA_h
#define COMPACTRPC_TINYPB_DATA_h

#include <stdint.h>
#include <vector>
#include <string>
#include "compactrpc/net/abstract_data.h"
#include "compactrpc/comm/log.h"

namespace compactrpc
{
    class TinyPbStruct : public AbstractData
    {
    public:
        typedef std::shared_ptr<TinyPbStruct> pb_ptr;

        TinyPbStruct() = default;
        ~TinyPbStruct() = default;
        TinyPbStruct(const TinyPbStruct &) = default;
        TinyPbStruct &operator=(const TinyPbStruct &) = default;
        TinyPbStruct(TinyPbStruct &&) = default;

        int32_t pk_len{0};      //  all length of package;
        int32_t msg_req_len{0}; // length of msg_req;
        std::string msg_req;
        int32_t service_name_len;
        std::string service_full_name; // like. QueryService.query_name;
        int32_t err_code{0};           // 0---call rpc succ, otherwise--call rpc failed, it only be seted by RpcController
        int32_t err_info_len;
        std::string err_info;
        std::string pb_data; // pb data
        int32_t check_num;   // check_num of all package, to check legality of data
    };
}
#endif