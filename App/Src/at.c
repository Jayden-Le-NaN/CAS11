#include "at.h"
#include "list.h"
#include "stdarg.h"
#include "string.h"
#include "stm32l4xx_hal.h"

//------------------------------------------------------------
// Created Time     : 2024.11.13
// Author           : JaydenLee
// Reference        : https://gitee.com/smtian/AT-Command
//------------------------------------------------------------

// 超时判断
#define AT_IS_TIMEOUT(start, time) (AT_GET_TICK() - (start) > (time))

//------------------------------AT作业类型------------------------------
#define AT_TYPE_WORK        0               // 普通作业
#define AT_TYPE_CMD         1               // 标准命令
#define AT_TYPE_MULTILIN    2               // 多行命令
#define AT_TYPE_SINGLLING   3               // 单行命令

#ifndef AT_DEBUG
    #define AT_DEBUG(...) do {} while(0)
#endif 

typedef int (*base_work)(at_obj_t* at, ...);

static void at_send_line(at_obj_t* at, const char* fmt, va_list args);

static const inline at_adapter_t* __get_adapter(at_obj_t* at) {
    return &at->adap;
}

static bool at_is_timeout(at_obj_t* at, uint32_t ms) {
    return AT_IS_TIMEOUT(at->timer, ms);
}

/*
 * @brief               发送数据
 * @param at            at结构体
 * @param buf           需要发送的数据
 * @param len           发送数据的长度
 * @return              无
 */
static void send_data(at_obj_t* at, const void* buf, uint32_t len) {
    at->adap.write(buf, len);
}

/*
 * @brief               格式化打印
 * @param at            at结构体
 * @param cmd           指令
 * @param ...           可变参数
 * @return              无
 */
static void print(at_obj_t* at, const char* cmd, ...) {
    va_list args;
    va_start(args, cmd);
    at_send_line(at, cmd, args);
    va_end(args);
}

/*
 * @brief               获取当前数据接收长度
 * @param at            at结构体
 * @return              当前数据接收长度
 */
static uint32_t get_recv_count(at_obj_t* at) {
    return at->recv_cnt;
}

/*
 * @brief               获取数据缓冲区
 * @param at            at结构体
 * @return              数据缓冲区
 */
static char* get_recv_buf(at_obj_t* at) {
    return (char*)at->adap.recv_buf;
}

/*
 * @brief               清除数据缓冲区
 * @param at            at结构体
 * @return              无
 */
static void recv_buf_clear(at_obj_t* at) {
    at->recv_cnt = 0;
}

/*
 * @brief               查找指定的字符串
 * @param at            at结构体
 * @param str           需要查找的字符串
 * @return              查找到的字符串的首位
 *
 */
static char* search_string(at_obj_t* at, const char* str) {
    return strstr(get_recv_buf(at), str);
}


/*
 * @brief               终止执行
 * @param at            at结构体
 * @return              终止执行的状态
 */
static bool at_isabort(at_obj_t* at) {
    return at->cursor ? at->cursor->abort : 1;
}

/*
 * @brief               重置计时器
 * @param at            at结构体
 * @return              无
 */
static void at_reset_timer(at_obj_t* at) {
    at->timer = AT_GET_TICK();
}

/*
 * @brief               AT执行回调
 * @param at            at结构体
 * @param item          at作业项
 * @param cb            at回调函数
 * @param ret           at响应类型
 * @return              无
 */
static void do_at_callback(at_obj_t* at, at_item_t* item, at_callback_t cb, at_return ret) {
    at_respose_t r;
    if ((ret == AT_RET_ERROR || ret == AT_RET_TIMEOUT) && at->adap.error != NULL)
        at->adap.error();

    if (cb) {
        r.param = item->param;
        r.recvbuf = get_recv_buf(at);
        r.recvcnt = get_recv_count(at);
        r.ret = ret;
        cb(&r);
    }
}


/*
 * @brief               添加作业到队列
 * @param at            at结构体
 * @param params        附加参数
 * @param info          附加类型
 * @param type          附加参数的类型
 * @return              是否成功添加作业
 */
static bool add_work(at_obj_t* at, void* params, void* info, int type) {
    at_item_t* item;
    if (list_empty(&at->ls_idle))                                   // 无空闲的at作业项
        return NULL;
    item = list_first_entry(&at->ls_idle, at_item_t, node);         // 从空闲链中取出作业
    item->info = (void*)info;
    item->param = (void*)params;
    item->state = AT_STATE_WAIT;
    item->type = type;
    item->abort = 0;
    list_move_tail(&item->node, &at->ls_ready);
    return item != 0;
}

/*
 * @brief               执行作业
 * @params at           at结构体
 */
static int do_work_handler(at_obj_t* at) {
    at_item_t* item = at->cursor;
    return ((int (*)(at_env_t* e))item->info)(&at->env);
}

/*
 * @brief               通用命令处理
 * @param at            at结构体
 * @return              true - 结束作业, false - 保持作业
 */
static int do_cmd_handler(at_obj_t* at) {
    at_item_t *item = at->cursor;       // at作业项
    at_env_t* env = &at->env;           // at环境

    const at_cmd_t* cmd = (at_cmd_t *)item->info;
    switch (env->state) {               // 空闲状态
        case AT_STATE_IDLE: 
            cmd->sender(env);
            env->state++;
            env->reset_timer(at);
            env->recvclr(at);           // 清空接收缓冲区
            break;
        case AT_STATE_WAIT:             
            if (search_string(at, cmd->mathcer)) {      // 接收匹配
                AT_DEBUG("<-\r\n%s\r\n", get_recv_buf(at));
                do_at_callback(at, item, cmd->cb, AT_RET_OK);
                return true;
            }
            else if (search_string(at, "ERROR")) {
                AT_DEBUG("<-\r\n%s\r\n", get_recv_buf(at));
                if (++env->i >= cmd->retry) {
                    do_at_callback(at, item, cmd->cb, AT_RET_ERROR);
                    return true;
                }
                env->state = AT_STATE_EXEC;             // 出错之后延时一段时间
                env->reset_timer(at);                   // 重置定时器
            }
            else if (env->is_timeout(at, cmd->timeout)) {       
                if (++env->i >= cmd->retry) {
                    do_at_callback(at, item, cmd->cb, AT_RET_TIMEOUT);
                    return true;
                }
                env->state = AT_STATE_IDLE;
            }
            break;
        case AT_STATE_EXEC:
            if (env->is_timeout(at, 500))               // 返回初始状态
                env->state = AT_STATE_IDLE;
            break;
        default:
            env->state = AT_STATE_IDLE;
    }
    return false;
}

/*
 * @brief               单行命令处理
 * @param at            at 结构体
 * @return              true - 结束作业, false - 保持作业
 */
static int send_singleline_handler(at_obj_t* at) {
    at_item_t* item = at->cursor;
    at_env_t* env = &at->env;
    const char* cmd = (const char*)item->param;
    at_callback_t cb = (at_callback_t)item->info;
    
    switch (env->state) {
        case AT_STATE_IDLE:
            env->printf(at, cmd);
            env->state++;
            env->reset_timer(at);
            env->recvclr(at);
        break;
        case AT_STATE_WAIT:
            if (search_string(at, "OK")) {
                AT_DEBUG("<-\r\n%s\r\n", get_recv_buf(at));
                do_at_callback(at, item, cb, AT_RET_OK);    // 接收匹配
                return true;
            }
            else if (search_string(at, "ERROR")) {
                AT_DEBUG("<-\r\n%s\r\n", get_recv_buf(at));
                if (++env->i >= 3) {
                    do_at_callback(at, item, cb, AT_RET_ERROR);
                    return true;
                }
                env->state = AT_STATE_EXEC;                 // 出错之后延时一段时间
                env->reset_timer(at);                       // 重置定时器
            }
            else if (env->is_timeout(at, 3000 + env->i * 2000)) {
                if (++env->i >= 3) {
                    do_at_callback(at, item, cb, AT_RET_TIMEOUT);
                    return true;
                }
                env->state = AT_STATE_IDLE;                 // 返回上一状态
            }
        break;
        case AT_STATE_EXEC:
            if (env->is_timeout(at, 500))
                env->state = AT_STATE_IDLE;
        break;
        default:
            env->state = AT_STATE_IDLE;
    }
    return false;
}

/*
 * @brief               多行命令管理
 * @param at            at 结构体
 * @return              true - 结束作业, false - 保持作业
 */
static int send_multiline_handler(at_obj_t* at) {
    at_item_t* item = at->cursor;
    at_env_t* env = &at->env;
    const char** cmds = (const char**)item->param;
    at_callback_t cb = (at_callback_t)item->info;

    switch (env->state) {
        case AT_STATE_IDLE:
            if (cmds[env->i] == NULL) {         // 命令执行完毕
                do_at_callback(at, item, cb, AT_RET_OK);
                return true;
            }
            env->printf(at, "%s\r\n", cmds[env->i]);
            env->recvclr(at);                   // 清除接收
            env->reset_timer(at);
            env->state++;
        break;
        case AT_STATE_WAIT:
            if (search_string(at, "OK")) {
                env->state = AT_STATE_IDLE;
                env->i++;
                env->j = 0;
                AT_DEBUG("<-\r\n%s\r\n", get_recv_buf(at));
            }
            else if (search_string(at, "ERROR")) {
                AT_DEBUG("<-\r\n%s\r\n", get_recv_buf(a));
                if (++env->j >= 3) {
                    do_at_callback(at, item, cb, AT_RET_ERROR);
                    return true;
                }
                env->state = AT_STATE_EXEC;     // 出错之后延时一段时间
                env->reset_timer(at);           // 重置定时器
            }
            else if (env->is_timeout(at, 3000)) {
                do_at_callback(at, item, cb, AT_RET_TIMEOUT);
                return true;
            }
        break;
        case AT_STATE_EXEC:
            if (env->is_timeout(at, 500)) 
                env->state = AT_STATE_IDLE;     // 返回初始状态
        break;
        default:
            env->state = AT_STATE_IDLE;
    }
    return false;
}

/*
 * @brief               发送行
 * @param fmt           格式化输出
 * @param args          可变参数列表
 * @returen             无
 */
static void at_send_line(at_obj_t* at, const char* fmt, va_list args) {
    char buf[AT_MAX_CMD_LEN];
    uint32_t len;
    const at_adapter_t* adt = __get_adapter(at);
    
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    
    recv_buf_clear(at);
    send_data(at, buf, len);
    send_data(at, "\r\n", 2);

    AT_DEBUG("->\r\n%s\r\n", buf);
}

/*
 * @brief               urc 处理总入口
 * @param at            at 结构体
 * @param urc           接收缓冲区
 * @param size          缓冲区大小
 * @return              无
 *
 */
static void urc_handler_entry(at_obj_t* at, char* urc, uint32_t size) {
    uint32_t i, n;
    urc_item_t* tbl = at->adap.urc_tbl;
    for (i = 0; i < at->adap.urc_tbl_count; ++i, ++tbl) {
        n = strlen(tbl->prefix);
        if (strncmp(urc, tbl->prefix, n) == 0) {            // 匹配前缀
            tbl->handler(urc, size);                        // 回调处理
            break;
        }
    }
    if (at->cursor == NULL)
        AT_DEBUG("<=\r\n%s\r\n", urc);
}

/*
 * @brief               urc 接收处理
 * @param buf           数据接收缓冲区
 * @return              无
 */
static void urc_recv_progress(at_obj_t* at, char* buf, uint32_t size) {
    char* urc_buf;
    int ch;
    uint16_t urc_size;
    urc_buf = (char*)at->adap.urc_buf;
    urc_size = at->adap.urc_buf_size;
    
    if (size == 0 && at->urc_cnt > 0) {
        if (AT_IS_TIMEOUT(at->urc_timer, 2000))  {          // 接收超时
            urc_handler_entry(at, urc_buf, at->urc_cnt);
            if (at->urc_cnt > 1)
                AT_DEBUG("Urc recv timeout.\r\n");
            at->urc_cnt = 0;
        }
    }
    else if (urc_buf != NULL) {
        at->urc_timer = AT_GET_TICK();
        while (size--) {
            ch = *buf++;
            urc_buf[at->urc_cnt++] = ch;
            if (ch == '\n' || ch == '\r' || ch == '\0') {   // urc 结束符
                urc_buf[at->urc_cnt] = '\0';
                if (at->urc_cnt > 2) 
                    urc_handler_entry(at, urc_buf, at->urc_cnt);
                at->urc_cnt = 0;
            }
            else if (at->urc_cnt >= urc_size)               // 溢出处理
                at->urc_cnt = 0;
        }
    }
}

/*
 * @brief               指令响应接收处理
 * @param at            at结构体
 * @param buf           接收到的数据
 * @param size          接收到的数据的长度
 * @return              无
 */
static void resp_recv_process(at_obj_t* at, const char* buf, uint32_t size) {
    char* recv_buf;
    uint16_t recv_size;
    
    recv_buf = (char*)at->adap.recv_buf;
    recv_size = at->adap.recv_buf_size;
    
    if (at->recv_cnt + size >= recv_size)           // 接收溢出处理
        at->recv_cnt = 0;
    
    memcpy(recv_buf + at->recv_cnt, buf, size);
    at->recv_cnt += size;
    recv_buf[at->recv_cnt] = '\0';
}

/*
 * @brief               AT控制器初始化
 * @param at            at结构体
 * @param adap          at适配器
 * @return              无
 */
 void at_obj_init(at_obj_t* at, const at_adapter_t* adap) {
    at_env_t* env;
    at->adap = *adap;
    env = &at->env; 
    at->recv_cnt = 0;
    
    env->is_timeout = at_is_timeout;
    env->printf = print;
    env->recvbuf = get_recv_buf;
    env->recvclr = recv_buf_clear;
    env->recvlen = get_recv_count;
    env->find = search_string;
    env->abort = at_isabort;
    env->reset_timer = at_reset_timer;

    // 链表初始化
    INIT_LIST_HEAD(&at->ls_idle);
    INIT_LIST_HEAD(&at->ls_ready);

    for (uint32_t i = 0; i < sizeof(at->items) / sizeof(at_item_t); ++i) 
        list_add_tail(&at->items[i].node, &at->ls_idle);

    while (adap->recv_buf == NULL) {}       // 确保缓冲区为非空
 }

/*
 * @brief               执行AT作业(自定义作业)
 * @param at            at结构体
 * @param work          AT 作业入口
 * @param params        参数
 * @return              是否成功添加作业
 */
bool at_do_work(at_obj_t* at, int (*work)(at_env_t* e), void* params) {
    return add_work(at, params, (void*)work, AT_TYPE_WORK);
}

/*
 * @brief               执行AT作业(自定义作业)
 * @param at            at结构体
 * @param params        参数
 * @param cmd           cmd命令
 * @return              是否成功添加作业
 */
bool at_do_cmd(at_obj_t* at, void* params, const at_cmd_t* cmd) {
    return add_work(at, params, (void*)cmd, AT_TYPE_CMD);
}

/*
 * @brief               执行AT作业(自定义作业)
 * @param at            at结构体
 * @param cb            执行回调
 * @param singleline    单行命令
 * @return              是否成功添加作业
 */
bool at_send_singleline(at_obj_t* at, at_callback_t cb, const char* singleline) {
    return add_work(at, (void*)singleline, (void*)cb, AT_TYPE_SINGLLING);
}

/*
 * @brief               执行AT作业(自定义作业)
 * @param at            at结构体
 * @param cb            执行回调
 * @param multiline     多行命令
 * @return              是否成功添加作业
 */
bool at_send_multiline(at_obj_t *at, at_callback_t cb, const char **multiline) {
    return add_work(at, multiline, (void*)cb, AT_TYPE_MULTILIN);
}

/*
 * @brief               强项终止AT作业
 * @param item          at 作业项
 * @return              无
 */
void at_item_abort(at_item_t* item) {
    item->abort = 1;
}

/*
 * @brief               AT 忙判断
 * @return              true - 有AT指令或者任务正在执行中
 */
bool at_obj_busy(at_obj_t* at) {
    return !list_empty(&at->ls_ready);
}

/*
 * @brief               AT 作业管理
 * @param at            at 结构体
 * @return              无
 */
static void at_work_manager(at_obj_t* at) {
    at_env_t* env = &at->env;
    //------------------------------作业处理表------------------------------
    static int (*const work_handler_table[])(at_obj_t*) = {
        do_work_handler,
        do_cmd_handler,
        send_multiline_handler,
        send_singleline_handler
    };

    if (at->cursor == NULL) {
        if (list_empty(&at->ls_ready))          // 就绪链为空
            return;
        env->i = 0;
        env->j = 0;
        env->state = AT_STATE_IDLE;
        at->cursor = list_first_entry(&at->ls_ready, at_item_t, node);
        env->params = at->cursor->param;
        env->recvclr(at);
        env->reset_timer(at);
    }

    //------------------------------工作执行完成,把他放到空闲工具链中------------------------------
    if (work_handler_table[at->cursor->type](at) || at->cursor->abort) {
        list_move_tail(&at->cursor->node, &at->ls_idle);
        at->cursor = NULL;
    }
}

/*
 * @brief               AT 轮训任务
 * @param at            at 结构体
 * @return              无
 */
void at_poll_taks(at_obj_t *at) {
    char rbuf[32];
    uint32_t read_size;
    read_size = __get_adapter(at)->read(rbuf, sizeof(rbuf));
    urc_recv_progress(at, rbuf, read_size);
    resp_recv_process(at, rbuf, read_size);
    at_work_manager(at);
}
