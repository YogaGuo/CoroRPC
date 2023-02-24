/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-12 21:44:35
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-14 21:58:57
 */
#ifndef COMPACTRPC_TINYPB_CODEC_H
#define COMPACTRPC_TINYPB_CODEC_H

#include <stdint.h>
#include "compactrpc/net/abstract_codec.h"
#include "compactrpc/net/abstract_data.h"
#include "compactrpc/net/tinypb/tinypb_data.h"

namespace compactrpc
{
    class TinyPbCodeC : public AbstractCodeC
    {
    public:
        TinyPbCodeC();
        ~TinyPbCodeC();

        void encode(TcpBuffer *buf, AbstractData *data) override;

        void decode(TcpBuffer *buf, AbstractData *data) override;

        ProtocalType getProtocalType() override;

        const char *encodePbData(TinyPbStruct *data, int &len);
    };
}

#endif