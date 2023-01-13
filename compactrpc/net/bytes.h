/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-13 14:35:52
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-13 14:41:39
 */
#ifndef COPMACTRPC_NET_BYTES_H
#define COPMACTRPC_NET_BYTES_H
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
namespace copmactrpc
{
    int32_t getInt32FromNetByte(const char *buf)
    {
        int32_t tmp;
        memcpy(&tmp, buf, sizeof(tmp));
        return ntohl(tmp);
    }
}
#endif