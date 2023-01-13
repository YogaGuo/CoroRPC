/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-12 21:11:28
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-12 21:50:13
 */
#ifndef COMPACTRPC_ABSTRACT_DATA_H
#define COMPACTRPC_ABSTRACT_DATA_H

namespace compactrpc
{

    class AbstractData
    {
    public:
        AbstractData() = default;
        virtual ~AbstractData(){};

        bool decode_succ{false};
        bool encode_succ{false};
    };
}

#endif