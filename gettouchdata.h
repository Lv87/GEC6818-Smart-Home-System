#ifndef _GETTOUCHDATA_H_
#define _GETTOUCHDATA_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/input.h>
#include <strings.h>
#include <errno.h>
#include <stdbool.h>

// 定义枚举类型
enum motion { left = 1, right, up, down };

struct touch_data
{
    unsigned int x;
    unsigned int y;
    enum motion m;
};
// 获取屏幕左划右划操作与坐标
struct touch_data get_touch_data(void);

#endif