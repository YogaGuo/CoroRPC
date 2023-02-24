/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-10 11:16:10
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-21 18:34:29
 */
#ifndef COMPACTRPC_RUN_TIME_H
#define COMPACTRPC_RUN_TIME_H

#include <string>

namespace compactrpc
{

    class RunTime
    {
    public:
        std::string m_msg_no;
        std::string m_interface_name;
    };

}

#endif