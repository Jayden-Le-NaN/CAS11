#ifndef AT_H_
#define AT_H_

#include "utils.h"
#include "list.h"
#include "stdbool.h"

#define AT_MAX_CMD_LEN          128

struct at_obj;                                                              // at结构体的前向声明

//------------------------------urc处理项------------------------------
typedef struct {
    const char *prefix;                                                     // 需要匹配的头部
    void (*handler)(char* recvbuf, int32_t len);                            // 执行函数
}urc_item_t;

//------------------------------AT接口适配器------------------------------
typedef struct {
    uint32_t    (*write)(const void* buf, uint32_t len);                    // 发送接口
    uint32_t    (*read)(void* buf, uint32_t len);                           // 接收接口
    void        (*error)(void);                                             // AT执行异常事件
    urc_item_t* urc_tbl;                                                    // urc 表
    uint8_t*    urc_buf;                                                    // urc接收缓冲区
    uint8_t*    recv_buf;                                                   // 数据缓冲区
    uint16_t    urc_tbl_count;                                              // urc表项个数
    uint16_t    recv_buf_size;                                              // 接收缓冲区大小
}at_adapter_t;

//------------------------------AT作业运行环境------------------------------
typedef struct {
    int         i ,j ,state;
    void*       params;
    void        (*reset_timer)(struct at_obj* at);                          // 重置时间
    bool        (*is_timeout)(struct at_obj* at, uint32_t ms);              // 时间跨度判断
    void        (*printf)(struct at_obj* at, const char* fmt, ...);         // printf 函数
    uint8_t*    (*find)(struct at_obj* at, const uint8_t* expect);          // 查找函数
    uint8_t*    (*recvbuf)(struct at_obj* at);                              // 指向接收缓冲区
    uint32_t    (*recvlen)(struct at_obj* at);                              // 缓冲区总长度
    void        (*recvclr)(struct at_obj* at);                              // 清空接收缓冲区
    bool        (*abort)(struct at_obj* at);                                // 终止执行
}at_env_t;

//------------------------------AT命令响应码------------------------------
typedef enum {
    AT_RET_OK = 0,                                                          // 执行成功
    AT_RET_ERROR,                                                           // 执行错误
    AT_RET_TIMEOUT,                                                         // 响应超时
    AT_RET_ABORT,                                                           // 强行中止
}at_return;

//------------------------------AT响应------------------------------
typedef struct {
    void*       param;                                                      
    uint8_t*    recvbuf;                                                    // 接收缓冲区
    uint16_t    recvcnt;                                                    // 接收数据长度
    at_return   ret;                                                        // AT 执行结果
}at_respose_t;

typedef void (*at_callback_t)(at_respose_t* r);                             // AT 执行回调

//------------------------------AT状态------------------------------
typedef enum {
    AT_STATE_IDLE,                                                          // 空闲状态
    AT_STATE_WAIT,                                                          // 等待状态
    AT_STATE_EXEC,                                                          // 执行状态
}at_work_state;



#endif
