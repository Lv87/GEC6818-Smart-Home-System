#include "gettouchdata.h"
// 该c文件是用于获取LCD屏幕的手势（上下左右）与屏幕坐标

// 获取屏幕左划右划操作与屏幕坐标
struct touch_data get_touch_data(void)
{
	// 1，打开触摸屏文件
	int tp = open("/dev/input/event0", O_RDONLY);
	if (tp == -1)
	{
		perror("open touch panel");
		exit(-1);
	}
	// 存放坐标与手势动作
	struct touch_data ts_data;
	// 2，读取触摸屏信息  input_event结构体在linux/input.h库中
	struct input_event buf;
	int x1 = 0, x2 = 0; // x1为按下屏幕x的起始值 ，x2为松开屏幕x的结束值
	int y1 = 0, y2 = 0; // y1为按下屏幕x的起始值 ，y2为松开屏幕x的结束值

	int xdone = 0;
	int ydone = 0;
	while (1)
	{
		bzero(&buf, sizeof(buf));
		read(tp, &buf, sizeof(buf)); // 读取触摸屏数据，放到 buf 中

		// 读到按键事件（包括键盘、触摸屏的按压、鼠标的左右键……）
		if (buf.type == EV_KEY)
		{
			// 读到触摸屏的按压事件
			if (buf.code == BTN_TOUCH)
			{
				// 读到按下触摸屏
				if (buf.value > 0)
				{
					xdone = 0;
					ydone = 0;
				}

				// 读到松开触摸屏
				if (buf.value == 0)
					break;
			}
		}

		// 读到触摸屏的坐标事件
		if (buf.type == EV_ABS)
		{
			if (xdone == 0 || ydone == 0)
			{
				if (buf.code == ABS_X)
				{
					x1 = buf.value * 800 / 1024; // 起始
					xdone = 1;
				}
				if (buf.code == ABS_Y)
				{
					y1 = buf.value * 480 / 600; // 起始
					ydone = 1;
				}
			}
			if (xdone == 1 || ydone == 1)
			{
				if (buf.code == ABS_X)
					x2 = buf.value * 800 / 1024; // 结束
				if (buf.code == ABS_Y)
					y2 = buf.value * 480 / 600; // 结束
			}
		}
	}

	// 将结束值作为坐标
	printf("起始x1 = %d 结束x2 = %d\n", x1, x2);
	printf("起始y1 = %d 结束y2 = %d\n", y1, y2);
	ts_data.x = x2;
	ts_data.y = y2;

	int dif_x = x1 - x2 > 0 ? x1 - x2 : x2 - x1;
	int dif_y = y1 - y2 > 0 ? y1 - y2 : y2 - y1;

	if (x1 == 0 || x2 == 0)
		ts_data.m = 0;

	if (x1 > x2 && dif_x > dif_y)
	{
		printf("向左滑动\n");
		ts_data.m = left;
	}
	else if (x1 < x2 && dif_x > dif_y)
	{
		printf("向右滑动\n");
		ts_data.m = right;
	}
	else if (y1 > y2 && dif_y > dif_x)
	{
		printf("向上滑动\n");
		ts_data.m = up;
	}
	else if (y1 < y2 && dif_y > dif_x)
	{
		printf("向下滑动\n");
		ts_data.m = down;
	}
	else
		ts_data.m = 0;

	// 3，释放资源
	close(tp);
	return ts_data;
}