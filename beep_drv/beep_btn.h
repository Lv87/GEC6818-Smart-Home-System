#ifndef _BEEP_BTN_H_
#define _BEEP_BTN_H_ 1

#include <linux/ioctl.h>

// 定义一个幻数,标识
#define GEC6818_BEEP_BTN_TYPE 'H'

// 定义命令,利用宏初始化命令
//_IO(type,nr):用于构造无参数的命令号
//_IOR(type,nr,datetype):用于构造从驱动程序中读取数据的命令号
//_IOW(type,nr,datatype):用于构造向驱动程序写入数据的命令号
//_IORW(type,nr,datatype):用于构造双向传输的命令号

// GEC6818_BEEP_PWM_SET 是设置PWM值,开启蜂鸣器
// GEC6818_BEEP_PWM_OFF 是关蜂鸣器

// 0,1,2为命令号
#define GEC6818_BEEP_PWM_OFF _IO(GEC6818_BEEP_BTN_TYPE, 0)
#define GEC6818_BEEP_PWM_SET _IOW(GEC6818_BEEP_BTN_TYPE, 1, int)

// 命令个数
#define GEC6818_BEEP_BTN_MAXNUM 2

struct pwm_value
{
    unsigned int arr; // 重装载值
    unsigned int ccr; // 比较值
    unsigned int key; // 有效电平极性
};

#endif