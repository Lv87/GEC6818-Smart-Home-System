#include "my_thread.h"

void *led_ctrl_routine(void *data)
{
    lcd_show_pic("image/InteractiveInterface/led_ctrl.bmp");
    printf("enter led contol\n");
    int fd;
    int ret;
    struct led_t
    {
        char buf[2]; // led.buf[1]---哪一盏灯：7/8/9/10 ,led.buf[0]---灯的状态：1--on，0--off
        int setKey;  // 已设置1,未设置0
    };
    struct led_t led;
    fd = open("/dev/led_dev", O_RDWR);
    if (fd < 0)
    {
        perror("open led dev");
        return;
    }
    while (!stop_flag)
    {
        memset(&led, 0, sizeof(struct led_t));
        if (90 < ts_xy.x && ts_xy.x < 260)
        {
            if (95 < ts_xy.y && ts_xy.y < 165) // LED1 on
            {
                led.buf[0] = 1;
                led.buf[1] = 7;
                led.setKey = 1;
            }
            else if (195 < ts_xy.y && ts_xy.y < 265) // LED1 off
            {
                led.buf[0] = 0;
                led.buf[1] = 7;
                led.setKey = 1;
            }
            else if (295 < ts_xy.y && ts_xy.y < 365) // LED2 on
            {
                led.buf[0] = 1;
                led.buf[1] = 8;
                led.setKey = 1;
            }
            else if (395 < ts_xy.y && ts_xy.y < 465) // LED2 off
            {
                led.buf[0] = 0;
                led.buf[1] = 8;
                led.setKey = 1;
            }
        }
        else if (525 < ts_xy.x && ts_xy.x < 690)
        {
            if (95 < ts_xy.y && ts_xy.y < 165) // LED3 on
            {
                led.buf[0] = 1;
                led.buf[1] = 9;
                led.setKey = 1;
            }
            else if (195 < ts_xy.y && ts_xy.y < 265) // LED3 off
            {
                led.buf[0] = 0;
                led.buf[1] = 9;
                led.setKey = 1;
            }
            else if (295 < ts_xy.y && ts_xy.y < 365) // LED4 on
            {
                led.buf[0] = 1;
                led.buf[1] = 10;
                led.setKey = 1;
            }
            else if (395 < ts_xy.y && ts_xy.y < 465) // LED4 off
            {
                led.buf[0] = 0;
                led.buf[1] = 10;
                led.setKey = 1;
            }
        }
        if (led.setKey)
        {
            ret = write(fd, led.buf, sizeof(led.buf));
            if (ret != 2)
            {
                perror("write led value");
                break;
            }
        }
        sleep(1);
    }
    close(fd);
    return;
}

void *lcd_check_routine(void *data)
{
    printf("checking LCD\n");
    lcd_show_color(RED); // RED实参
    sleep(1);
    lcd_show_color(GREEN);
    sleep(1);
    lcd_show_flag(FRANCE);
    sleep(1);
    lcd_show_flag(GERNARY);
    while (!stop_flag)
        ;
    return;
}

void *madplay_routine(void *data)
{
    printf("music_command:%s\n", (const char *)data);
    system((const char *)data);
    printf("music end\n");
    return;
}

void *music_routine(void *data)
{
    lcd_show_pic("image/InteractiveInterface/music.bmp");
    printf("enter music playing\n");
    system("find music/*.mp3 > music/musicPath.txt");
    system("echo end >> music/musicPath.txt");
    pthread_t madplay_branch;
    char music_command[1024];
    int ret;
    // 初始化链表
    linklist head = init_list();
    // 遍历目录内mp3信息并插入链表中
    head = write_path("music/musicPath.txt", ".mp3", "end", head);
    // 操作mp3节点的指针
    linklist tmp = head->next;

    while (!stop_flag)
    {
        memset(music_command, 0, sizeof(char[1024]));
        strcpy(music_command, "madplay ");
        if (200 < ts_xy.y && ts_xy.y < 270) // 歌曲内操作
        {
            if (25 < ts_xy.x && ts_xy.x < 175) // 播放
            {
                strcat(music_command, tmp->path_name);
                ret = pthread_create(&madplay_branch, NULL, madplay_routine, music_command);
                if (ret == 0)
                {
                    printf("create madplay_branch ok \n");
                }
            }
            else if (225 < ts_xy.x && ts_xy.x < 380) // 暂停
            {
                system("killall -STOP madplay");
            }
            else if (420 < ts_xy.x && ts_xy.x < 570) // 继续
            {
                system("killall -CONT madplay");
            }
            else if (615 < ts_xy.x && ts_xy.x < 765) // 停止
            {
                system("killall -KILL madplay");
            }
        }
        else if (310 < ts_xy.y && ts_xy.y < 380) // 曲库操作
        {
            if (130 < ts_xy.x && ts_xy.x < 280) // 上一首
            {
                system("killall -KILL madplay");
                tmp = tmp->prev;
                if (tmp == head)
                    tmp = head->prev;
                strcat(music_command, tmp->path_name);
                ret = pthread_create(&madplay_branch, NULL, madplay_routine, music_command);
                if (ret == 0)
                {
                    printf("create madplay_branch ok \n");
                }
            }
            else if (510 < ts_xy.x && ts_xy.x < 660) // 下一首
            {
                system("killall -KILL madplay");
                tmp = tmp->next;
                if (tmp == head)
                    tmp = head->next;
                strcat(music_command, tmp->path_name);
                ret = pthread_create(&madplay_branch, NULL, madplay_routine, music_command);
                if (ret == 0)
                {
                    printf("create madplay_branch ok \n");
                }
            }
        }
        sleep(1);
    }
    system("killall -KILL madplay");
    return;
}

void *beep_ctrl_routine(void *data)
{
    lcd_show_pic("image/InteractiveInterface/beep_ctrl.bmp");
    printf("enter beep ctrl\n");
    int fd, len = 0;
    struct pwm_value PWM;
    fd = open("/dev/beep_dev", O_RDWR);
    if (fd < 0)
    {
        perror("open beep dev");
        return;
    }
    while (!stop_flag)
    {
        memset(&PWM, 0, sizeof(struct pwm_value));
        if (190 < ts_xy.y && ts_xy.y < 330)
        {
            if (120 < ts_xy.x && ts_xy.x < 320) // beep on
            {
                PWM.arr = 300;
                PWM.ccr = 150;
                PWM.key = 1;
                len = ioctl(fd, GEC6818_BEEP_PWM_SET, &PWM);
            }
            else if (480 < ts_xy.x && ts_xy.x < 690) // beep off
            {
                len = ioctl(fd, GEC6818_BEEP_PWM_OFF);
            }
        }
        sleep(1);
    }
    close(fd);
    return;
}

int photo_key = 1;
void *photo_display_routine(void *data)
{
    linklist head = (linklist)data;
    linklist temp = head->next;
    while (temp != head)
    {
        // 每隔一秒显示下一张
        lcd_show_pic(temp->path_name);
        sleep(1);
        temp = temp->next;
    }
    printf("图片播放完毕\n");
    // 结束后 显示回 主页
    lcd_show_pic(temp->next->path_name);
    photo_key = 1;
    return;
}

void *photo_routine(void *data)
{
    system("find image/photo/*.bmp > image/photo/imagePath.txt");
    system("echo end >> image/photo/imagePath.txt");
    pthread_t photo_display_branch;
    // 初始化链表
    linklist head = init_list();
    // 遍历目录内图片信息并插入链表中
    head = write_path("image/photo/imagePath.txt", ".bmp", "end", head);
    // 操作图片节点的指针
    linklist tmp = head->next;
    lcd_show_pic(tmp->path_name);
    while (!stop_flag)
    {
        // 获取屏幕手势操作
        if (ts_xy.m == left && photo_key)
        {
            tmp = tmp->next;
            if (tmp == head)
                tmp = head->next;
            printf("显示下一张图片\n");
            lcd_show_pic(tmp->path_name);
        }
        else if (ts_xy.m == right && photo_key)
        {
            tmp = tmp->prev;
            if (tmp == head)
                tmp = head->prev;
            printf("显示上一张图片\n");
            lcd_show_pic(tmp->path_name);
        }
        else if (ts_xy.m == up && photo_key)
        {
            printf("自动播放图片\n");
            tmp = head;
            photo_key = pthread_create(&photo_display_branch, NULL, photo_display_routine, tmp);
            if (photo_key == 0)
            {
                printf("create photo_display_branch ok \n");
            }
            tmp = tmp->next;
        }
        else if (ts_xy.m == down && (!photo_key))
        {
            pthread_cancel(photo_display_branch);
            pthread_join(photo_display_branch, NULL);
            photo_key = 1;
            printf("停止播放\n");
        }
        sleep(1);
    }
    if (!photo_key)
    {
        pthread_cancel(photo_display_branch);
        pthread_join(photo_display_branch, NULL);
        printf("停止播放\n");
    }
    printf("photo end\n");
    return;
}

void *key_ctrl_routine(void *data)
{
    lcd_show_pic("image/InteractiveInterface/key.bmp");
    printf("enter key contro\n");
    while (!stop_flag)
        ;
    return;
}
