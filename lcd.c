#include "lcd.h"

int lcd_show_color(unsigned int color)
{
	int ret = 0;
	int x, y;
	int fd_lcd;
	fd_lcd = open("/dev/fb0", O_RDWR);
	if (fd_lcd == -1)
	{
		perror("open lcd");
		ret = fd_lcd;
		return ret;
	}

	int *lcd_base = NULL;
	lcd_base = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd_lcd, 0);
	if (lcd_base == NULL)
	{
		perror("lcd mmap");
		goto lcd_mmap_failed;
	}

	for (y = 0; y < 480; y++)
		for (x = 0; x < 800; x++)
			*(lcd_base + 800 * y + x) = color;

	munmap(lcd_base, 800 * 480 * 4);
lcd_mmap_failed:
	close(fd_lcd);
	return ret;
}

int lcd_show_flag(unsigned int country)
{
	int ret = 0;
	int x, y;
	int fd_lcd;
	fd_lcd = open("/dev/fb0", O_RDWR);
	if (fd_lcd == -1)
	{
		perror("open lcd");
		ret = fd_lcd;
		return ret;
	}

	int *lcd_base = NULL;
	lcd_base = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd_lcd, 0);
	if (lcd_base == NULL)
	{
		perror("lcd mmap");
		ret = -1;
		goto lcd_mmap_failed;
	}

	for (y = 0; y < 480; y++)
		for (x = 0; x < 800; x++)
			if (country == FRANCE)
			{
				if (x < 266)
					*(lcd_base + x + y * 800) = 0x0000ff00;
				else if (x < 534)
					*(lcd_base + x + y * 800) = 0x00ffffff;
				else
					*(lcd_base + x + y * 800) = 0x00ff0000;
			}
			else if (country == GERNARY)
			{
				if (y < 160)
					*(lcd_base + x + y * 800) = 0x00000000;
				else if (y < 320)
					*(lcd_base + x + y * 800) = 0x00ff0000;
				else
					*(lcd_base + x + y * 800) = 0x00ffff00;
			}

lcd_mmap_failed:
	close(fd_lcd);
	return ret;
}

int lcd_show_pic(char *bmp_name)
{
	int ret = 0;
	char image_buf[800 * 480 * 3] = {0}; // 相当于bmp图片
	int fd, image;
	int *lcd_base;
	int x, y;
	fd = open("/dev/fb0", O_RDWR);
	if (-1 == fd)
	{
		perror("open lcd");
		ret = fd;
		return ret;
	}
	image = open(bmp_name, O_RDONLY);
	if (-1 == image)
	{
		perror("open image");
		ret = image;
		goto open_image_error;
	}
	lcd_base = mmap(NULL, 800 * 480 * 4, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (lcd_base == NULL)
	{
		perror("lcd mmap");
		ret = -1;
		goto mmap_lcd_error;
	}
	lseek(image, 54, SEEK_SET); // 对图片的文件信息进行过滤，不是有效的色彩值
	read(image, image_buf, 800 * 480 * 3);
	for (y = 0; y < 480; y++)
	{
		for (x = 0; x < 800; x++)
		{
			// 将3个字节数据赋值给4字节
			*(lcd_base + x + (479 - y) * 800) = image_buf[3 * (x + y * 800) + 0] | image_buf[3 * (x + y * 800) + 1] << 8 | image_buf[3 * (x + y * 800) + 2] << 16 | 0x00 << 24;
		}
	}
	munmap(lcd_base, 800 * 480 * 4);
	// 5.关闭文件
mmap_lcd_error:
	close(image);
open_image_error:
	close(fd);
	return ret;
}
