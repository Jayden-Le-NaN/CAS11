#include "ringbuffer.h"

//------------------------------------------------------------
// @Created Time:       2024.11.16
// @Author      :       JaydenLee
//------------------------------------------------------------
#define min(a, b) (a < b) ? (a) : (b)

/*
 * @brief               构造一个环形缓冲区
 * @param[in] r         环形缓冲区管理器
 * @param[in] buffer    数据缓冲区
 * @param[in] size      数据缓冲区长度(需要为2的N次幂)
 * @return              true : 缓冲区不为NULL并且长度为2的N次幂
 */
bool ring_buffer_init(ring_buffer_t* r, uint8_t* buffer, uint32_t size) {
    r->buffer = buffer;
    r->size = size;
    r->front = 0;
    r->rear = 0;
    return (buffer != NULL) && ((size & size - 1) == 0);
}


/*
 * @brief               清除环形缓冲区
 * @param[in] r         环形缓冲区管理器
 * @return              无
 */
void ring_buffer_clear(ring_buffer_t* r) {
    r->front = 0;
    r->rear = 0;
}

/*
 * @brief               获取环形缓冲区的数据长度
 * @return              环形缓冲区的有效字节数
 */
uint32_t ring_buffer_len(ring_buffer_t* r) { 
    return r->rear - r->front;              // 没有必要做处理,一般不会用超过4G的内存在单片机上
}

/*
 * @brief               将指定长度的数据放到环形缓冲区中
 * @param[in] r         环形缓冲区管理器
 * @param[in] buffer    输入数据缓冲区
 * @param[in] len       缓冲区长度
 * @return              实际存放到环形缓冲区中的数据长度
 */
uint32_t ring_buffer_put(ring_buffer_t* r, uint8_t* buffer, uint32_t len) {
    uint32_t i;
    uint32_t leave = r->size + r->front - r->rear;
    len = min(leave, len);
    i = min(len, r->size - (r->rear & r->size - 1));        // 因为环形缓冲区的大小是2的N次幂, 所以r->rear & r->size - 1是一个取模操作
    memcpy(r->buffer + (r->rear & r->size - 1), buffer, i);
    memcpy(r->buffer, buffer + i, len - i);
    r->rear += len;
    return len;
}

/*
 * @brief               从环形缓冲区中取出指定长度的数据
 * @param[in]           环形缓冲区管理器
 * @param[out]          输出数据缓冲区
 * @param[in]           读取数据长度
 * @return              实际读取长度
 */
uint32_t ring_buffer_get(ring_buffer_t* r, uint8_t* buffer, uint32_t len) {
    if (r->rear == r->front)
        return 0;

    uint32_t i;
    uint32_t leave = r->rear - r->front;
    len = min(len, leave);
    i = min(len, r->size - (r->front & r->size - 1));
    memcpy(buffer, r->buffer + (r->front & r->size - 1), i);
    memcpy(buffer + i, r->buffer, len - i);
    r->front += len;
    return len;
}
