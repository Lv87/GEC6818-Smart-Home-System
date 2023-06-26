#ifndef _LCD_H_ // 条件编译的作用
#define _LCD_H_ 1

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#define RED 0x00FF0000
#define GREEN 0x0000FF00
#define BLUE 0x000000FF
#define FRANCE 1
#define GERNARY 2

// 填充颜色用于液晶屏检测
int lcd_show_color(unsigned int color);
// 显示旗帜用于液晶屏检测
int lcd_show_flag(unsigned int country);
// 显示bmp
int lcd_show_pic(char *bmp_name);
#endif /*!_LCD_H_*/
