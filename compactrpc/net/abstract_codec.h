/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-12 21:11:43
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-12 22:05:45
 */
#ifndef COMPACTRPC_ABSTRACT_CODEC_H
#define COMPACTRPC_ABSTRACT_CODEC_H

#include <memory>
#include <string>
#include "compactrpc/net/tcp/tcp_buffer.h"
#include "compactrpc/net/abstract_data.h"
namespace compactrpc
{
    enum ProtocalType
    {
        TinyPb_Protocal = 1,
        Http_Protocal = 2
    };

    class AbstractCodec
    {
    public:
        std::shared_ptr<AbstractCodec> ptr;
        AbstractCodec() {}
        virtual ~AbstractCodec();
        /**
         * @brief :��AbstractData��ʽ���ݰ����������buf��
         *
         * @param buf
         * @param data
         */
        virtual void encode(TcpBuffer *buf, AbstractData *data) = 0;

        /**
         * @brief ���� buf �н����һ��AbstractData��ʽ�����ݰ�
         *
         * @param buf
         * @param data
         */
        virtual void decode(TcpBuffer *buf, AbstractData *data) = 0;
        virtual ProtocalType getProtocalType() = 0;
    };
}
#endif