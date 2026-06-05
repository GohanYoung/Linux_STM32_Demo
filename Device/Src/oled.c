#include "oled.h"
#include "bsp_i2c.h" // 绝不包含 HAL，只调我们的 BSP 接口
#include "oled_font.h"// 引入字库头文件

/* 底层写命令函数 (内部使用，不暴露给外部) */
static void OLED_WriteCmd(uint8_t cmd) {
    // SSD1306 规定：写命令时，内存地址(控制字节)为 0x00
    BSP_I2C1_WriteMem(OLED_I2C_ADDR, 0x00, &cmd, 1);
}

/* 底层写数据函数 (内部使用) */
static void OLED_WriteData(uint8_t data) {
    // SSD1306 规定：写显存数据时，内存地址(控制字节)为 0x40
    BSP_I2C1_WriteMem(OLED_I2C_ADDR, 0x40, &data, 1);
}

/* OLED 初始化序列 (SSD1306 数据手册规定) */
void Device_OLED_Init(void) {
    OLED_WriteCmd(0xAE); // 关闭显示
    OLED_WriteCmd(0x20); // 设置内存寻址模式
    OLED_WriteCmd(0x02); // 00,水平寻址; 01,垂直寻址; 10,页寻址(默认)
    OLED_WriteCmd(0xB0); // 为页寻址模式设置页起始地址,0-7
    OLED_WriteCmd(0xC8); // 设置 COM 输出扫描方向
    OLED_WriteCmd(0x00); // 设置低列地址
    OLED_WriteCmd(0x10); // 设置高列地址
    OLED_WriteCmd(0x40); // 设置起始行地址
    OLED_WriteCmd(0x81); // 设置对比度控制寄存器
    OLED_WriteCmd(0xFF); // 亮度调节 0x00~0xff
    OLED_WriteCmd(0xA1); // 设置线段重映射
    OLED_WriteCmd(0xA6); // 正常显示/反显 (0xA7为反显)
    OLED_WriteCmd(0xA8); // 设置多路复用率(1 to 64)
    OLED_WriteCmd(0x3F); // 1/32 duty (适配128x32屏幕)
    OLED_WriteCmd(0xD3); // 设置显示偏移
    OLED_WriteCmd(0x00); // 不偏移
    OLED_WriteCmd(0xD5); // 设置显示时钟分频比例/振荡器频率
    OLED_WriteCmd(0x80); // 设置分频比
    OLED_WriteCmd(0xD9); // 设置预充电周期
    OLED_WriteCmd(0x22); 
    OLED_WriteCmd(0xDA); // 设置 COM 硬件引脚配置
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0xDB); // 设置 vcomh
    OLED_WriteCmd(0x20); // 0x20,0.77xVcc
    OLED_WriteCmd(0x8D); // 设置电荷泵
    OLED_WriteCmd(0x14); // 开启电荷泵
    OLED_WriteCmd(0xAF); // 开启显示
    Device_OLED_Clear(); // 清屏
}

/* 清除屏幕 */
void Device_OLED_Clear(void) {
    uint8_t i, n;
    for(i = 0; i < 8; i++) {  
        OLED_WriteCmd(0xB0 + i); // 设置页地址（0~7）
        OLED_WriteCmd(0x00);     // 设置显示位置—列低地址
        OLED_WriteCmd(0x10);     // 设置显示位置—列高地址   
        for(n = 0; n < 128; n++) {
            OLED_WriteData(0);   // 全局写 0 熄灭像素
        }
    }
}

static void OLED_SetPos(uint8_t x, uint8_t y) {
    OLED_WriteCmd(0xB0 + y);                 // 设置页地址 (Page)
    OLED_WriteCmd(((x & 0xF0) >> 4) | 0x10); // 设置列高位地址
    OLED_WriteCmd((x & 0x0F) | 0x00);        // 设置列低位地址
}

/* * 对外 API：在指定位置显示一个字符 (8x16大小)
 */
void Device_OLED_ShowChar(uint8_t x, uint8_t y, char chr) {
    uint8_t i;
    // 获取字符在字库中的索引
    uint8_t font_idx = Get_Font_Index(chr); 
    
    // 防越界保护：如果 X 轴画不下了，自动换行到下一页 (y+2)
    if (x > 120) { x = 0; y += 2; } 

    // 1. 画字符的上半部分 (前 8 个字节)
    OLED_SetPos(x, y); 
    for(i = 0; i < 8; i++) {
        OLED_WriteData(F8X16[font_idx][i]);
    }
    
    // 2. 画字符的下半部分 (后 8 个字节)
    OLED_SetPos(x, y + 1); 
    for(i = 0; i < 8; i++) {
        OLED_WriteData(F8X16[font_idx][i + 8]);
    }
}

/* * 对外 API：显示字符串
 */
void Device_OLED_ShowString(uint8_t x, uint8_t y, char *chr) {
    uint8_t j = 0;
    while (chr[j] != '\0') { // 遇到字符串结束符 '\0' 停止
        Device_OLED_ShowChar(x, y, chr[j]);
        x += 8; // 每个字符宽 8 个像素，光标向右移 8
        if (x > 120) { 
            x = 0;   // 换行
            y += 2;  // 8x16 字符高度占 2 个页
        }
        j++;
    }
}