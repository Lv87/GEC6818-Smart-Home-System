#ifndef _DIRLINKLIST_H_
#define _DIRLINKLIST_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

// 定义链表存放path
typedef struct linklist
{
	char path_name[1024];
	struct linklist *next;
	struct linklist *prev;
} listnode, *linklist;

// write path to list    path:path.txt key:路径特征值 end:结束位
linklist write_path(const char *path_name, const char *key, const char *end, linklist head);

// 初始化链表
linklist init_list(void);

// 创建新节点
linklist creat_node(char *path_name);

// 插入链表末尾
void insert_add_tail(linklist new, linklist head);

// 遍历链表
void show_list(linklist head);
#endif