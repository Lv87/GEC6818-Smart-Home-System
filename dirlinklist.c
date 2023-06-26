#include "dirlinklist.h"
// 该c文件为链表的相关操作函数，双向循环链表用于存放path信息。

// 初始化链表
linklist init_list(void)
{
	linklist head = calloc(1, sizeof(listnode));
	head->next = head;
	head->prev = head;
	return head;
}

// 创建新节点
linklist creat_node(char *path_name)
{
	linklist new = calloc(1, sizeof(listnode));

	// 切记 字符串赋值用strcpy ！！！！！
	strcpy(new->path_name, path_name);
	new->next = new;
	new->prev = new;
	return new;
}

// 插入链表末尾
void insert_add_tail(linklist new, linklist head)
{
	new->next = head;
	new->prev = head->prev;

	head->prev->next = new;
	head->prev = new;
}

// 遍历链表
void show_list(linklist head)
{
	linklist tmp = head->next;
	while (tmp != head)
	{
		printf("%s", tmp->path_name);
		tmp = tmp->next;
	}
	printf("\n");
}

// 打开path.txt,将信息并插入链表中
linklist write_path(const char *path_name, const char *key, const char *end, linklist head)
{
	int Path_fd;
	int count = 0;
	char c;
	char buff[1024];
	Path_fd = open(path_name, O_RDWR);
	if (-1 == Path_fd)
	{
		perror("open path file");
		exit(-1);
	}
	while (read(Path_fd, &c, 1) != -1)
	{
		if (c == '\n')
		{
			buff[count] = '\0';
			if (strstr(buff, end) != NULL)
				break;
			if (strstr(buff, key) != NULL)
			{
				// 创建链表新节点存放路径
				linklist new = creat_node(buff);
				// 将新节点插入到链表末尾
				insert_add_tail(new, head);
			}
			count = 0;
			memset(buff, 0, sizeof(char[1024]));
			continue;
		}
		buff[count] = c;
		count++;
	}
	close(Path_fd);
	// 返回链表头
	return head;
}