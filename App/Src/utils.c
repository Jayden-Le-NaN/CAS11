#include "utils.h"

//------------------------------重定向printf------------------------------
extern UART_HandleTypeDef huart1;

void printf(const char *format, ...)
{
    char buffer[PRINT_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, PRINT_BUFFER_SIZE, format, args);
    va_end(args);

    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, sizeof(buffer), HAL_MAX_DELAY);
}

/*
 * @brief               使能GPIO的时钟
 * @param GPIOx         GPIO的类型
 * @return              UTILS_OK: 没问题
 *                      UTILS_ERROR: 时钟类型传输错误
 */
UTILS_Status UTILS_RCC_GPIO_Enable(GPIO_TypeDef* GPIOx) {
    UTILS_Status status = UTILS_OK;

    if (GPIOx == GPIOA)     
        __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (GPIOx == GPIOB)
        __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (GPIOx == GPIOC)
        __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (GPIOx == GPIOD)
        __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (GPIOx == GPIOE)
        __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (GPIOx == GPIOF)
        __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (GPIOx == GPIOH)
        __HAL_RCC_GPIOH_CLK_ENABLE();
    else
        status = UTILS_ERROR;

    return status;
}

/*
 * @brief               以微秒级延时
 * @param delay         延时的时间
 * @return              无
 */
void UTILS_Delay_us(__IO uint32_t delay) {
    // 系统嘀嗒定时器计完一次数是1ms
    int32_t last, curr, val;
    int32_t tmp;
    while (delay != 0) {
        tmp = delay > 900 ? 900 : delay;
        last = SysTick->VAL;
        curr = last - MCU_FREQUENCY_MHZ * tmp;
        if (curr >= 0) {
            do {
                val = SysTick->VAL;
            }while((val < last) && (val >= curr));
        }
        else {
            curr += MCU_FREQUENCY_MHZ * 1000;
            do {
                val = SysTick->VAL;
            }while((val <= last) || (val) > curr);
        }
        delay -= tmp;
    }
}

/*
 * @brief               设置字节指定比特位的数值
 * @param byte          需要设置的字节
 * @param bit_pos       需要设置的比特位置
 * @param bit_state     需要设置的数据大小
 * @return              程序运行状态
 * @note                建议使用提供宏函数 UTILS_WriteBit
 */
UTILS_Status UTILS_WriteBit_Byte(uint8_t* byte, uint8_t bit_pos, UTILS_BitState bit_state) {
    UTILS_Status status = UTILS_OK;
    do {
        // 检查提供的位置数据是否合理
        if (bit_pos > 7) {
            status = UTILS_ERROR;
            break;
        }

        if (bit_state == UTILS_BIT_SET) 
            *byte |= (1 << bit_pos);
        else
            *byte &= ~(1 << bit_pos);
    } while(0);
    return status;
}

/*
 * @brief               设置字指定比特位的数值
 * @param word          需要设置的字
 * @param bit_pos       需要设置的比特位置
 * @param bit_state     需要设置的数据大小
 * @return              程序运行状态
 * @note                建议使用提供宏函数 UTILS_WriteBit
 */
UTILS_Status UTILS_WriteBit_Word(uint16_t* word, uint8_t bit_pos, UTILS_BitState bit_state) {
    UTILS_Status status = UTILS_OK;
    do {
        if (bit_pos > 15) {
            status = UTILS_ERROR;
            break;
        }
        if (bit_state == UTILS_BIT_SET) 
            *word |= (1 << bit_pos);
        else
            *word &= ~(1 << bit_pos);
    } while(0);
    return status;
}

/*
 * @brief               设置指定区域的比特为指定的值
 * @param byte          需要设置的字节
 * @param msb           目标位的高位索引
 * @param lsb           目标位的地位索引
 * @param value         需要更改的值
 * @return              程序运行状态
 * @note                建议直接使用提供的宏函数 UTILS_WriteBit_Zone
 */
UTILS_Status UTILS_WriteBit_Zone_Byte(uint8_t* byte, uint8_t msb, uint8_t lsb, uint8_t value) {
    UTILS_Status status = UTILS_OK;
    do {
        if (msb > 7 || lsb > 7 || msb < lsb) {
            status = UTILS_ERROR;
            break;
        }
        uint8_t mask = ((1 << (msb - lsb + 1)) - 1) << lsb;
        *byte &= ~mask;
        *byte |= (value << lsb) & mask;
    } while(0);
    return status;
}

/*
 * @brief               设置指定区域的比特为指定的值
 * @param byte          需要设置的字
 * @param msb           目标位的高位索引
 * @param lsb           目标位的地位索引
 * @param value         需要更改的值
 * @return              程序运行状态
 * @note                建议直接使用提供的宏函数 UTILS_WriteBit_Zone
 */
UTILS_Status UTILS_WriteBit_Zone_Word(uint16_t* word, uint8_t msb, uint8_t lsb, uint16_t value) {
    UTILS_Status status = UTILS_OK;
    do {
        if (msb > 7 || lsb > 7 || msb < lsb) {
            status = UTILS_ERROR;
            break;
        }
        uint8_t mask = ((1 << (msb - lsb + 1)) - 1) << lsb;
        *word &= ~mask;
        *word |= (value << lsb) & mask;
    } while(0);
    return status;
}

/*
 * @brief               向上取整
 * @param data          需要取整的函数
 * @return              取整结果
 */
int32_t UTILS_Ceil(double data) {
    if (data < 0)
        return (int32_t)data;

    int32_t int_part = (int32_t)data;
    if (data > int_part)
        return (int_part + 1);
    return int_part;
}

int32_t UTILS_Log2(uint32_t value) {
    if (value == 0)
        return - 1;
    return 31 - __builtin_clz(value);
}


