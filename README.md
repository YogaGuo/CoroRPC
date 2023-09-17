<!--
 * @Description: 
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-09 13:45:19
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-09 14:27:11
-->
###  C++11实现的基于协程的异步高性能RPC框架

```
    CoroRPC
    ├── compactrpc
    │   ├── comm
    │   │   ├── config.cpp
    │   │   ├── config.h
    │   │   ├── error_code.h
    │   │   ├── log.cpp
    │   │   ├── log.h
    │   │   ├── msg_req.cpp
    │   │   ├── msg_req.h
    │   │   ├── Mutex.cpp
    │   │   ├── Mutex.h
    │   │   ├── run_time.cpp
    │   │   ├── run_time.h
    │   │   ├── string_util.cpp
    │   │   └── string_util.h
    │   ├── conf
    │   │   ├── test_http_server.xml
    │   │   └── test_tinypb_server.xml
    │   ├── coroutine
    │   │   ├── coctx.h
    │   │   ├── coctx_swap.S
    │   │   ├── coroutine.cpp
    │   │   ├── coroutine.h
    │   │   ├── coroutine_hook.cpp
    │   │   ├── coroutine_hook.h
    │   │   ├── coroutine_pool.cpp
    │   │   ├── coroutine_pool.h
    │   │   ├── memory.cpp
    │   │   └── memory.h
    │   ├── makefile
    │   └── net
    │       ├── abstract_codec.h
    │       ├── abstract_data.h
    │       ├── abstract_dispatch.h
    │       ├── bytes.h
    │       ├── fd_event.cpp
    │       ├── fd_event.h
    │       ├── http
    │       │   ├── http_codec.cpp
    │       │   ├── http_codec.h
    │       │   ├── http_define.cpp
    │       │   ├── http_define.h
    │       │   ├── http_request.h
    │       │   └── http_response.h
    │       ├── net_address.cpp
    │       ├── net_address.h
    │       ├── reactor.cpp
    │       ├── reactor.h
    │       ├── tcp
    │       │   ├── abstract_slot.h
    │       │   ├── io_thread.cpp
    │       │   ├── io_thread.h
    │       │   ├── tcp_buffer.cpp
    │       │   ├── tcp_buffer.h
    │       │   ├── tcp_client.cpp
    │       │   ├── tcp_client.h
    │       │   ├── tcp_connection.cpp
    │       │   ├── tcp_connection.h
    │       │   ├── tcp_conn_time_wheel.cpp
    │       │   ├── tcp_conn_time_wheel.h
    │       │   ├── tcp_server.cpp
    │       │   └── tcp_server.h
    │       ├── timer.cpp
    │       ├── timer.h
    │       └── tinypb
    │           ├── tinypb_codec.cpp
    │           ├── tinypb_codec.h
    │           ├── tinypb_data.h
    │           ├── tinypb_rpc_channel.cpp
    │           ├── tinypb_rpc_channel.h
    │           ├── tinypb_rpc_closure.cpp
    │           ├── tinypb_rpc_closure.h
    │           ├── tinypb_rpc_controller.cpp
    │           ├── tinypb_rpc_controller.h
    │           ├── tinypb_rpc_dispatcher.cpp
    │           └── tinypb_rpc_dispatcher.h
    └── README.md
```