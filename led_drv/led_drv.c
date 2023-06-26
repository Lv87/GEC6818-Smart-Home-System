// 头文件：/kernel/linux/*.h
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/io.h>

#define DEV_NAME "led_dev" // /dev/led_dev

#define setBit(num, pos) ((num) |= (1 << (pos)))  // num的pos位置1
#define clrBit(num, pos) ((num) &= ~(1 << (pos))) // num的pos位清0
#define rvslBit(num, pos) ((num) ^= (1 << (pos))) // num的pos位反转

#define GPIOxBASE 0xC0010000
#define GPIOCBASE (GPIOxBASE + 0x0000C000)
#define GPIOEBASE (GPIOxBASE + 0x0000E000)

static dev_t led_num;			   // 设备号
static unsigned int led_major = 0; // 主设备号：0--动态分配，not 0 ---静态注册
static unsigned int led_minor = 0;

// 1.定义一个cdev
static struct cdev led_dev;

static struct class *gec6818_led_class;
static struct device *gec6818_led_dev;

static struct resource *led_res;
static void __iomem *led_res_gpioc;
static void __iomem *led_res_gpioe;

struct gpio_t
{
	volatile uint32_t GPIOxOUT;						 // GPIOx output register
	volatile uint32_t GPIOxOUTENB;					 // GPIOx output enable register
	volatile uint32_t GPIOxDETMODE0;				 // GPIOx event detect mode register 0
	volatile uint32_t GPIOxDETMODE1;				 // GPIOx event detect mode register 1
	volatile uint32_t GPIOxINTENB;					 // GPIOx interrupt enable register
	volatile uint32_t GPIOxDET;						 // GPIOx event detect register
	volatile uint32_t GPIOxPAD;						 // GPIOx PAD status register
	volatile uint32_t RSVD;							 // Reserved
	volatile uint32_t GPIOxALTFN0;					 // GPIOx alternate function select register 0
	volatile uint32_t GPIOxALTFN1;					 // GPIOx alternate function select register 1
	volatile uint32_t GPIOxDETMODEEX;				 // GPIOx event detect mode extended register
	volatile uint32_t GPIOxDETENB;					 // GPIOx detect enable register
	volatile uint32_t GPIOx_SLEW;					 // GPIOx slew register
	volatile uint32_t GPIOx_SLEW_DISABLE_DEFAULT;	 // GPIOx slew disable defaultregister
	volatile uint32_t GPIOx_DRV1;					 // GPIOx DRV1 register
	volatile uint32_t GPIOx_DRV1_DISABLE_DEFAULT;	 // GPIOx DRV1 disable default register
	volatile uint32_t GPIOx_DRV0;					 // GPIOx DRV0 register
	volatile uint32_t GPIOx_DRV0_DISABLE_DEFAULT;	 // GPIOx DRV0 disable default register
	volatile uint32_t GPIOx_PULLSEL;				 // GPIOx PULLSEL register
	volatile uint32_t GPIOx_PULLSEL_DISABLE_DEFAULT; // GPIOx PULLSEL disable default register
	volatile uint32_t GPIOx_PULLENB;				 // GPIOx PULLENB register
	volatile uint32_t GPIOx_PULLENB_DISABLE_DEFAULT; // GPIOx PULLENB disable default register
};

static struct gpio_t *GPIOC, *GPIOE;

// 3、定义一个文件操作集，并对其进行初始化
static int gec6818_led_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	// 8、申请物理内存区，作为一个资源
	led_res = request_mem_region(GPIOCBASE, 0x1000, "GPIOC_LED");
	if (led_res == NULL)
	{
		pr_err("%s: GPIOC_LED request mem region error\n", __func__);
		ret = -EBUSY;
		return ret;
	}

	led_res = request_mem_region(GPIOEBASE, 0x1000, "GPIOE_LED");
	if (led_res == NULL)
	{
		pr_err("%s: GPIOE_LED request mem region error\n", __func__);
		ret = -EBUSY;
		goto request_mem_gpioe_err;
	}

	// 9、IO内存动态映射，得到虚拟地址
	led_res_gpioc = ioremap(GPIOCBASE, 0x1000);
	if (led_res_gpioc == NULL)
	{
		pr_err("%s: GPIOC ioremap is error\n", __func__);
		ret = -EBUSY;
		goto ioremap_gpioc_err;
	}
	GPIOC = (struct gpio_t *)led_res_gpioc;

	led_res_gpioe = ioremap(GPIOEBASE, 0x1000);
	if (led_res_gpioe == NULL)
	{
		pr_err("%s: GPIOE ioremap is error\n", __func__);
		ret = -EBUSY;
		goto ioremap_gpioe_err;
	}
	GPIOE = (struct gpio_t *)led_res_gpioe;

	// 10、访问虚拟地址
	// 将GPIOC 7 8 17 GPIOE 13统一设置成普通的GPIO :  01--->function 1
	GPIOC->GPIOxALTFN1 &= ~(3 << 2);
	GPIOC->GPIOxALTFN1 |= (1 << 2); // GPIOC17

	GPIOC->GPIOxALTFN0 &= ~((3 << 16) + (3 << 14));
	GPIOC->GPIOxALTFN0 |= ((1 << 16) + (1 << 14)); // GPIOC7/8
	// GPIOE13 function 0
	GPIOE->GPIOxALTFN0 &= ~(3 << 26); // GPIOE13

	// 将将GPIOC 7 8 17 GPIOE 13配置成输出
	GPIOC->GPIOxOUTENB |= ((1 << 17) + (1 << 8) + (1 << 7));
	GPIOE->GPIOxOUTENB |= (1 << 13);

	// 将将GPIOC 7 8 17 GPIOE 13配置成输出高电平
	GPIOC->GPIOxOUT |= ((1 << 17) + (1 << 8) + (1 << 7));
	GPIOE->GPIOxOUT |= (1 << 13);

	printk("led driver is openning\n");
	return ret;

ioremap_gpioe_err:
	iounmap(led_res_gpioc);
ioremap_gpioc_err:
	release_mem_region(GPIOEBASE, 0x1000);
request_mem_gpioe_err:
	release_mem_region(GPIOCBASE, 0x1000);
	return ret;
}
static int gec6818_led_release(struct inode *inode, struct file *filp)
{
	iounmap(led_res_gpioc);
	iounmap(led_res_gpioe);
	release_mem_region(GPIOEBASE, 0x1000);
	release_mem_region(GPIOCBASE, 0x1000);
	printk("led driver is closing\n");
	return 0;
}
// 定义数据的格式：使用应用程序写下来的数据来控制LED灯：
// buf[1]--设置哪一盏灯：7、8、9、10
// buf[0]--设置灯的状态：1--on，0--off
static ssize_t gec6818_led_write(struct file *filp, const char __user *buf, size_t len, loff_t *ops)
{
	char kbuf[2];
	int ret;
	if (len != 2)
		return -EINVAL;
	ret = copy_from_user(kbuf, buf, len);
	if (ret != 0)
	{
		pr_err("%s: copy from user error\n", __func__);
		return -EFAULT;
	}
	// 得到应用程序下来的两个字节数，使用该数据控灯。
	if (kbuf[0] == 1) // LED on
		switch (kbuf[1])
		{
		case 7: // GPIOE13-->LED7
			GPIOE->GPIOxOUT &= ~(1 << 13);
			break;
		case 8: // GPIOC17-->LED8
			GPIOC->GPIOxOUT &= ~(1 << 17);
			break;
		case 9: // GPIOC8-->LED9
			GPIOC->GPIOxOUT &= ~(1 << 8);
			break;
		case 10: // GPIOC7-->LED10
			GPIOC->GPIOxOUT &= ~(1 << 7);
			break;
		default:
			return -EINVAL;
		}
	else if (kbuf[0] == 0) // LED off
		switch (kbuf[1])
		{
		case 7: // GPIOE13-->LED7
			GPIOE->GPIOxOUT |= (1 << 13);
			break;
		case 8: // GPIOC17-->LED8
			GPIOC->GPIOxOUT |= (1 << 17);
			break;
		case 9: // GPIOC8-->LED9
			GPIOC->GPIOxOUT |= (1 << 8);
			break;
		case 10: // GPIOC7-->LED10
			GPIOC->GPIOxOUT |= (1 << 7);
			break;
		default:
			return -EINVAL;
		}
	else
		return -EINVAL;

	return len;
}
static const struct file_operations gec6818_led_fops = {
	.owner = THIS_MODULE,
	.open = gec6818_led_open,
	.write = gec6818_led_write,
	.release = gec6818_led_release,
};

// 驱动的安装函数
static int __init gec6818_led_init(void)
{
	int ret = 0;

	// 2、申请设备号
	if (led_major == 0) // 动态分配
		ret = alloc_chrdev_region(&led_num, led_minor, 1, "led_device");
	else
	{ // 静态注册
		led_num = MKDEV(led_major, led_minor);
		ret = register_chrdev_region(led_num, 1, "led_device");
	}
	if (ret < 0)
	{
		pr_err("%s: can not get a led device number\n", __func__);
		return ret;
	}

	// 4、字符设备初始化
	cdev_init(&led_dev, &gec6818_led_fops);

	// 5、将cdev加入到内核
	ret = cdev_add(&led_dev, led_num, 1);
	if (ret < 0)
	{
		pr_err("%s: led cdev add failed\n", __func__);
		goto cdev_add_err;
	}

	// 6、创建class
	gec6818_led_class = class_create(THIS_MODULE, "led_class");
	if (gec6818_led_class == NULL)
	{
		pr_err("%s: led class create error\n", __func__);
		ret = -EBUSY;
		goto class_create_err;
	}

	// 7、创建device
	gec6818_led_dev = device_create(gec6818_led_class, NULL, led_num, NULL, DEV_NAME); // /dev/led_dev
	if (gec6818_led_dev == NULL)
	{
		pr_err("%s: led device create error\n", __func__);
		ret = -EBUSY;
		goto device_create_err;
	}

	printk("GEC6818 led device driver init\n");
	return ret;

device_create_err:
	class_destroy(gec6818_led_class);
class_create_err:
	cdev_del(&led_dev);
cdev_add_err:
	unregister_chrdev_region(led_num, 1);
	return ret;
}

// 驱动的卸载函数
static void __exit gec6818_led_exit(void)
{
	device_destroy(gec6818_led_class, led_num); // 销毁device
	class_destroy(gec6818_led_class);			// 删除class
	cdev_del(&led_dev);							// 删除cdev
	unregister_chrdev_region(led_num, 1);		// 释放设备号
	printk("GEC6818 led device driver exit\n");
}

// module的入口和出口
module_init(gec6818_led_init); // 安装驱动:#insmod led_drv.ko --->module_init()--->gec6818_led_init( )
module_exit(gec6818_led_exit); // 卸载驱动:#rmmod led_drv.ko --->module_exit()--->gec6818_led_exit()

// module的描述, #modinfo *.ko
MODULE_AUTHOR("bobeyfeng@163.com");
MODULE_DESCRIPTION("GEC6818: LED Device Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("V1.0");