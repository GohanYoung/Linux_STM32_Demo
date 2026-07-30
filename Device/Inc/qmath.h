#ifndef __QMATH_H
#define __QMATH_H

#include <stdint.h>

/* ====================================================
 * Q15 定点数类型定义 (Q16.15 格式)
 *
 * 位布局 (int32_t):
 *   [31]    符号位 (1 bit)
 *   [30:15] 整数位 (16 bits), 范围 [-65536, 65535]
 *   [14:0]  小数位 (15 bits), 精度 1/32768 ≈ 0.00003
 *
 * 缩放因子: 2^15 = 32768
 * ==================================================== */
typedef int32_t q15_t;

#define Q15_SCALE      15
#define Q15_FACTOR     32768.0f
#define Q15_FRAC_MASK  0x7FFF
#define Q15_ONE        ((q15_t)32768)       // 1.0
#define Q15_HALF       ((q15_t)16384)       // 0.5
#define Q15_TENTH      ((q15_t)3277)        // 0.1

/* --- 转换宏 --- */

/* float → Q15 */
#define Q15_FROM_FLOAT(f)   ((q15_t)((f) * Q15_FACTOR + ((f) >= 0 ? 0.5f : -0.5f)))

/* int → Q15 */
#define Q15_FROM_INT(i)     ((q15_t)((int32_t)(i) << Q15_SCALE))

/* Q15 → float (调试用) */
#define Q15_TO_FLOAT(q)     ((float)(q) / Q15_FACTOR)

/* Q15 → int (截断取整) */
#define Q15_TO_INT(q)       ((int16_t)((q) >> Q15_SCALE))

/* --- 四则运算 --- */

/* 乘法: Q15 × Q15 → Q15, 使用 int64_t 中间变量防止溢出 */
#define Q15_MUL(a, b)       ((q15_t)(((int64_t)(a) * (int64_t)(b)) >> Q15_SCALE))

/* 除法: Q15 ÷ Q15 → Q15 */
#define Q15_DIV(a, b)       ((q15_t)(((int64_t)(a) << Q15_SCALE) / (int64_t)(b)))

/* 加法 */
#define Q15_ADD(a, b)       ((q15_t)((a) + (q15_t)(b)))

/* 减法 */
#define Q15_SUB(a, b)       ((q15_t)((a) - (q15_t)(b)))

/* --- 显示辅助 --- */

/* 提取整数部分 */
#define Q15_INTEGER(q)      ((int16_t)((q) >> Q15_SCALE))

/* 提取 1 位小数 (用于 "XX.X" 格式显示) */
#define Q15_FRAC1(q)        ((int)((((q) & Q15_FRAC_MASK) * 10 + (Q15_FACTOR / 2)) >> Q15_SCALE))

/* --- 限幅 --- */
#define Q15_CLAMP(val, lo, hi)  \
    (((val) < (lo)) ? (lo) : (((val) > (hi)) ? (hi) : (val)))

#endif