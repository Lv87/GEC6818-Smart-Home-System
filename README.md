# GEC6818 Smart Home System
* ## 项目描述：基于GEC6818开发板智能家居控制系统
  * ### 项目内容：
    1. 液晶屏检测：在液晶屏上显示单色、彩条、方格、国旗
    2. 数码相册：在液晶屏上显示照片
    3. LED 控制：房间、客厅、厨房、洗手间
    4. 音乐播放：播放、暂停、继续、上一曲、下一曲
    5. 报警控制：控制蜂鸣器
  * ### 使用技术：
    1. C 语言
    2. 多线程
    3. 交叉编译
    4. 驱动开发
* ## 项目目录结构
  * main            (程序可执行文件)

  * led_drv.ko      (编译好的led驱动文件)

  * beep_drv.ko     (编译好的蜂鸣器驱动文件)

  * music           (MP3音频文件目录)

  * image           (BMP图片目录)

    * photo                     (相册目录)

    * InteractiveInterface      (交互控制界面目录)

        ![main.bmp](image/InteractiveInterface/main.bmp)

        ![beep_ctrl.bmp](image/InteractiveInterface/beep_ctrl.bmp)

        ![control.bmp](image/InteractiveInterface/control.bmp)

        ![key.bmp](image/InteractiveInterface/key.bmp)

        ![led_ctrl.bmp](image/InteractiveInterface/led_ctrl.bmp)

        ![music.bmp](image/InteractiveInterface/music.bmp)