#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif
    
// 环形缓冲区管理器
typedef struct {
    uint8_t         *buffer;    // 环形缓冲区
    uint32_t        size;       // 环形缓冲区的大小
    uint32_t        front;      // 头指针 
    uint32_t        rear;       // 尾指针
}ring_buffer_t;

bool ring_buffer_init(ring_buffer_t* r, uint8_t* buffer, uint32_t size);

void ring_buffer_clear(ring_buffer_t* r);

uint32_t ring_buffer_len(ring_buffer_t* r);

uint32_t ring_buffer_put(ring_buffer_t* r, uint8_t* buffer, uint32_t len);

uint32_t ring_buffer_get(ring_buffer_t* r, uint8_t* buffer, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif
