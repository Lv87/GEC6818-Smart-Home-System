#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lcd.h"
#include "gettouchdata.h"
#include "my_thread.h"

int main(void)
{
	int key = 1;
	stop_flag = 0;
	int flag;
	pthread_t led_ctrl_branch, lcd_check_branch, music_branch, beep_ctrl_branch, photo_branch, key_ctrl_branch;
	int ret;
	lcd_show_pic("image/InteractiveInterface/main.bmp");
	while (1)
	{
		ts_xy = get_touch_data();
		sleep(1);
		if (630 < ts_xy.x && ts_xy.x < 800 && 415 < ts_xy.y && ts_xy.y < 480)
		{
			// 进入系统
			lcd_show_pic("image/InteractiveInterface/control.bmp");
			while (1)
			{
				memset(&ts_xy, 0, sizeof(struct touch_data));
				ts_xy = get_touch_data();
				sleep(1);
				if (key)
				{
					if (60 < ts_xy.x && ts_xy.x < 310)
					{
						// 进入LED控制界面
						if (130 < ts_xy.y && ts_xy.y < 205)
						{
							ret = pthread_create(&led_ctrl_branch, NULL, led_ctrl_routine, NULL);
							if (ret == 0)
							{
								key = 0;
								flag = LED_CTRL;
								printf("create led_ctrl_branch ok \n");
							}
						}

						// 进入液晶屏检测界面
						else if (235 < ts_xy.y && ts_xy.y < 310)
						{
							ret = pthread_create(&lcd_check_branch, NULL, lcd_check_routine, NULL);
							if (ret == 0)
							{
								key = 0;
								flag = LCD_CHECK;
								printf("create lcd_check_branch ok \n");
							}
						}

						// 进入音乐播放界面
						else if (345 < ts_xy.y && ts_xy.y < 420)
						{
							ret = pthread_create(&music_branch, NULL, music_routine, NULL);
							if (ret == 0)
							{
								key = 0;
								flag = MUSIC;
								printf("create music_branch ok \n");
							}
						}
					}
					else if (460 < ts_xy.x && ts_xy.x < 715)
					{
						// 进入报警控制界面
						if (130 < ts_xy.y && ts_xy.y < 205)
						{
							ret = pthread_create(&beep_ctrl_branch, NULL, beep_ctrl_routine, NULL);
							if (ret == 0)
							{
								key = 0;
								flag = BEEP_CTRL;
								printf("create beep_ctrl_branch ok \n");
							}
						}

						// 进入数码相册界面
						else if (235 < ts_xy.y && ts_xy.y < 310)
						{
							ret = pthread_create(&photo_branch, NULL, photo_routine, NULL);
							if (ret == 0)
							{
								key = 0;
								flag = PHOTO;
								printf("create photo_branch ok \n");
							}
						}

						// 进入按键控制(家电控制)界面
						else if (345 < ts_xy.y && ts_xy.y < 420)
						{
							ret = pthread_create(&key_ctrl_branch, NULL, key_ctrl_routine, NULL);
							if (ret == 0)
							{
								key = 0;
								flag = KEY_CTRL;
								printf("create key_ctrl_branch ok \n");
							}
						}
					}
				}
				if (660 < ts_xy.x && ts_xy.x < 800 && 0 < ts_xy.y && ts_xy.y < 100)
				{
					// 返回主页面
					switch (flag)
					{
					case LED_CTRL:
						printf("stop led_ctrl_branch\n");
						stop_flag = 1;
						// pthread_cancel(led_ctrl_branch);
						pthread_join(led_ctrl_branch, NULL);
						break;
					case LCD_CHECK:
						printf("stop lcd_check_branch\n");
						stop_flag = 1;
						pthread_cancel(lcd_check_branch);
						pthread_join(lcd_check_branch, NULL);
						break;
					case MUSIC:
						printf("stop music_branch\n");
						stop_flag = 1;
						// pthread_cancel(music_branch);
						pthread_join(music_branch, NULL);
						break;
					case BEEP_CTRL:
						printf("stop beep_ctrl_branch\n");
						stop_flag = 1;
						// pthread_cancel(beep_ctrl_branch);
						pthread_join(beep_ctrl_branch, NULL);
						break;
					case PHOTO:
						printf("stop photo_branch\n");
						stop_flag = 1;
						// pthread_cancel(photo_branch);
						pthread_join(photo_branch, NULL);
						break;
					case KEY_CTRL:
						printf("stop key_ctrl_branch\n");
						stop_flag = 1;
						pthread_cancel(key_ctrl_branch);
						pthread_join(key_ctrl_branch, NULL);
						break;
					}
					key = 1;
					flag = 0;
					stop_flag = 0;
					printf("ok\n");
					lcd_show_pic("image/InteractiveInterface/control.bmp");
				}
			}
		}
	}
	return 0;
}
