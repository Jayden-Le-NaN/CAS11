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

//------------------------------AT作业项------------------------------
typedef struct {
    uint32_t    state : 3;                                                  // 作业类型
    uint32_t    type  : 3;                          
    uint32_t    abort : 1;
    void*       param;                                                      // 通用参数
    void*       info;                                                       // 通用信息指针
    struct list_head node;                                                  // 链表节点
}at_item_t;

//------------------------------AT管理器------------------------------
typedef struct at_obj {
    at_adapter_t            adap;                                           // at接口适配器
    at_env_t                env;                                            // 作业运行环境
    at_item_t               items[32];                                      // 最大容纳32个作业
    at_item_t               *cursor;                                        // 当前作业项
    struct list_head        ls_ready, ls_idle;                              // 就绪, 空闲作业链表
    uint32_t                timer;                                          // 计时器
    uint32_t                urc_timer;                                      // urc接收计时器
    at_return               ret;
    uint16_t                urc_cnt, recv_cnt;                              // urc接收计数, 命令响应接收计数器
    uint8_t                 suspend : 1;
}at_obj_t;

typedef struct {
    void (*sender)(at_env_t *e);                                            // 自定义发送器
    const char* mathcer;                                                    // 匹配字符串
    at_callback_t cb;                                                       // 响应处理
    uint8_t retry;                                                          // 错误重试次数
    uint16_t timeout;                                                       // 最大超时时间
}at_cmd_t;

void at_obj_init(at_obj_t* at, const at_adapter_t*);

bool at_send_singleline(at_obj_t* at, at_callback_t cb, const char* singlling);

bool at_send_multiling(at_obj_t* at, at_callback_t cb, const char** multiline);

bool at_do_cmd(at_obj_t* at, void* params, const at_cmd_t* cmd);

void at_item_abort(at_item_t* it);                                          // 终止当前作业

bool at_obj_busy(at_obj_t* at);                                             // 忙判断

void at_suspend(at_obj_t* at);

void at_resume(at_obj_t* at);

void at_poll_taks(at_obj_t* at);


#endif
