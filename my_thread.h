#ifndef _MY_THREAD_H_
#define _MY_THREAD_H_ 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "lcd.h"
#include "gettouchdata.h"
#include "beep_btn.h"
#include "dirlinklist.h"

int stop_flag;
struct touch_data ts_xy;

#define LED_CTRL 1
#define LCD_CHECK 2
#define MUSIC 3
#define BEEP_CTRL 4
#define PHOTO 5
#define KEY_CTRL 6

// 进入LED控制界面
void *led_ctrl_routine(void *data);

// 进入液晶屏检测界面
void *lcd_check_routine(void *data);

// 进入音乐播放界面
void *music_routine(void *data);

// 进入报警控制界面
void *beep_ctrl_routine(void *data);

// 进入数码相册界面
void *photo_routine(void *data);

// 进入按键控制(家电控制)界面
void *key_ctrl_routine(void *data);

#endif /* !_MY_THREAD_H_ */