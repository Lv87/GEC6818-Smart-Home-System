// 头文件：/kernel/linux/*.h
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <cfg_type.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>
#include <linux/types.h>
#include "beep_btn.h"

#define DEV_NAME "beep_dev" // /dev/beep_dev

#define setBit(num, pos) ((num) |= (1 << (pos)))  // num的pos位置1
#define clrBit(num, pos) ((num) &= ~(1 << (pos))) // num的pos位清0
#define rvslBit(num, pos) ((num) ^= (1 << (pos))) // num的pos位反转

#define GPIOxBASE 0xC0010000
#define GPIOCBASE (GPIOxBASE + 0x0000C000)
#define PWMBASE 0xC0018000

static dev_t beep_num;				// 设备号
static unsigned int beep_major = 0; // 主设备号：0--动态分配，not 0 ---静态注册
static unsigned int beep_minor = 0;

// 1.定义一个cdev
static struct cdev beep_dev;

static struct class *gec6818_beep_class;
static struct device *gec6818_beep_dev;

static struct resource *beep_res;
static void __iomem *beep_res_gpioc;
static void __iomem *beep_res_pwm;

struct pwm_t
{
	volatile uint32_t TCFG0;	  // Clock-Prescalar and Dead-Zone configurations
	volatile uint32_t TCFG1;	  // Clock multiplexers and DMA mode select
	volatile uint32_t TCON;		  // Timer control register
	volatile uint32_t TCNTB0;	  // Timer 0 count buffer register
	volatile uint32_t TCMPB0;	  // Timer 0 compare buffer register
	volatile uint32_t TCNTO0;	  // Timer 0 count observation register
	volatile uint32_t TCNTB1;	  // Timer 1 count buffer register
	volatile uint32_t TCMPB1;	  // Timer 1 compare buffer register
	volatile uint32_t TCNTO1;	  // Timer 1 count observation register
	volatile uint32_t TCNTB2;	  // Timer 2 count buffer register
	volatile uint32_t TCMPB2;	  // Timer 2 compare buffer register
	volatile uint32_t TCNTO2;	  // Timer 2 count observation register
	volatile uint32_t TCNTB3;	  // Timer 3 count buffer register
	volatile uint32_t TCMPB3;	  // Timer 3 compare buffer register
	volatile uint32_t TCNTO3;	  // Timer 3 count observation register
	volatile uint32_t TCNTB4;	  // Timer 4 count buffer register
	volatile uint32_t TCNTO4;	  // Timer 4 count observation register
	volatile uint32_t TINT_CSTAT; // Timer interrupt control and status register
};

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

static struct pwm_t *PWM;
static struct gpio_t *GPIOC;

// 3、定义一个文件操作集，并对其进行初始化
static int gec6818_beep_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	// 8、申请物理内存区，作为一个资源
	beep_res = request_mem_region(GPIOCBASE, 0x1000, "GPIOC_BEEP");
	if (beep_res == NULL)
	{
		pr_err("%s: GPIOC_BEEP request mem region error\n", __func__);
		ret = -EBUSY;
		return ret;
	}

	beep_res = request_mem_region(PWMBASE, 0x1000, "PWM_BEEP");
	if (beep_res == NULL)
	{
		pr_err("%s: PWM_BEEP request mem region error\n", __func__);
		ret = -EBUSY;
		goto request_mem_pwm_err;
	}

	// 9、IO内存动态映射，得到虚拟地址
	beep_res_gpioc = ioremap(GPIOCBASE, 0x1000);
	if (beep_res_gpioc == NULL)
	{
		pr_err("%s: GPIOC ioremap is error\n", __func__);
		ret = -EBUSY;
		goto ioremap_gpioc_err;
	}
	GPIOC = (struct gpio_t *)beep_res_gpioc;

	beep_res_pwm = ioremap(PWMBASE, 0x1000);
	if (beep_res_pwm == NULL)
	{
		pr_err("%s: PWM ioremap is error\n", __func__);
		ret = -EBUSY;
		goto ioremap_pwm_err;
	}
	PWM = (struct pwm_t *)beep_res_pwm;

	printk("beep driver is openning\n");
	return ret;

ioremap_pwm_err:
	iounmap(beep_res_gpioc);
ioremap_gpioc_err:
	release_mem_region(PWMBASE, 0x1000);
request_mem_pwm_err:
	release_mem_region(GPIOCBASE, 0x1000);
	return ret;
}
static int gec6818_beep_release(struct inode *inode, struct file *filp)
{
	// 复位寄存器
	PWM->TCNTB2 &= ~(0xffff << 0);
	PWM->TCMPB2 &= ~(0xffff << 0);
	PWM->TCON &= ~(0xf << 12);
	PWM->TCFG1 &= ~(0xf << 8);
	PWM->TCFG0 &= ~(0xff << 8);
	PWM->TCFG0 |= (0xff << 8);
	GPIOC->GPIOxALTFN0 &= ~(3 << 28);
	setBit(GPIOC->GPIOxOUTENB, 14);
	clrBit(GPIOC->GPIOxOUTENB, 14);

	iounmap(beep_res_pwm);
	iounmap(beep_res_gpioc);
	release_mem_region(PWMBASE, 0x1000);
	release_mem_region(GPIOCBASE, 0x1000);
	printk("beep driver is closing\n");
	return 0;
}
static long gec6818_beep_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct pwm_value pwm_v;
	int rc = 0;
	if (_IOC_NR(cmd) > GEC6818_BEEP_BTN_MAXNUM)
		return -EINVAL;
	if (_IOC_TYPE(cmd) != GEC6818_BEEP_BTN_TYPE)
		return -EINVAL;
	if (_IOC_DIR(cmd) == _IOC_READ)
	{
		if (!access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd)))
			return -EINVAL;
	}
	else
	{
		if (!access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd)))
			return -EINVAL;
	}
	switch (cmd)
	{
	case GEC6818_BEEP_PWM_SET:
		// 这里需要强制转换的类型为unsigned char * 保证数据按最小进行分割
		rc = copy_from_user(&pwm_v, (char __user *)arg, sizeof(struct pwm_value));
		if (rc)
		{
			pr_err("%s: copy from user error\n", __func__);
			return rc;
		}
		printk("%s: pwm set arr = %d, ccr = %d key = %d\n", __func__, pwm_v.arr, pwm_v.ccr, pwm_v.key);
		// 设置GPIOC14引脚为PWM功能  GPIOCALTFN0[29:28] = 0x10
		clrBit(GPIOC->GPIOxALTFN0, 28);
		clrBit(GPIOC->GPIOxALTFN0, 29);
		setBit(GPIOC->GPIOxALTFN0, 29);
		// 设置对PCLK时钟的一级分频值,进行250分频  TCFG0[15:8] = 249  600000HZ
		PWM->TCFG0 &= ~(0xff << 8);
		PWM->TCFG0 |= (249 << 8);
		// 设置对PCLK时钟的二级分频值，进行2分频  TCFG1[11:8] = 0x0001 300000HZ
		PWM->TCFG1 &= ~(0xf << 8);
		PWM->TCFG1 |= (0x0001 << 8);
		// 设置TCNTB2的初始值，确定PWM方波的最终周期
		PWM->TCNTB2 &= ~(0xffff << 0);
		PWM->TCNTB2 |= (pwm_v.arr - 1);
		// 设置TCMPB2的初始值值，确定PWM方波的占空比
		PWM->TCMPB2 &= ~(0xffff << 0);
		PWM->TCMPB2 |= (pwm_v.ccr - 1);
		// 清除TCON15:12
		PWM->TCON &= ~(0xf << 12);
		// 确定方波的初始状态
		if (pwm_v.key)
			setBit(PWM->TCON, 14); // 低电平
		else
			clrBit(PWM->TCON, 14); // 高电平
		// 打开手动更新
		setBit(PWM->TCON, 13);
		// 打开自动重载
		setBit(PWM->TCON, 15);
		// 关闭手动更新
		clrBit(PWM->TCON, 13);
		// 定时器2的使能
		setBit(PWM->TCON, 12);
		break;

	case GEC6818_BEEP_PWM_OFF:
		// 关闭自动重载
		clrBit(PWM->TCON, 15);
		// 定时器2关闭
		clrBit(PWM->TCON, 12);
		// 设置GPIOC14引脚为GPIOC14功能
		clrBit(GPIOC->GPIOxALTFN0, 28);
		clrBit(GPIOC->GPIOxALTFN0, 29);
		setBit(GPIOC->GPIOxALTFN0, 28);
		// 设置GPIOC14为输入模式0
		setBit(GPIOC->GPIOxOUTENB, 14);
		clrBit(GPIOC->GPIOxOUTENB, 14);
		break;

	default:
		return -EINVAL;
	}
	return rc;
}
static const struct file_operations gec6818_beep_fops = {
	.owner = THIS_MODULE,
	.open = gec6818_beep_open,
	.release = gec6818_beep_release,
	.unlocked_ioctl = gec6818_beep_ioctl,
};

// 驱动的安装函数
static int __init gec6818_beep_init(void)
{
	int ret = 0;

	// 2、申请设备号
	if (beep_major == 0) // 动态分配
		ret = alloc_chrdev_region(&beep_num, beep_minor, 1, "beep_device");
	else
	{ // 静态注册
		beep_num = MKDEV(beep_major, beep_minor);
		ret = register_chrdev_region(beep_num, 1, "beep_device");
	}
	if (ret < 0)
	{
		pr_err("%s: can not get a beep device number\n", __func__);
		return ret;
	}

	// 4、字符设备初始化
	cdev_init(&beep_dev, &gec6818_beep_fops);

	// 5、将cdev加入到内核
	ret = cdev_add(&beep_dev, beep_num, 1);
	if (ret < 0)
	{
		pr_err("%s: beep cdev add failed\n", __func__);
		goto cdev_add_err;
	}

	// 6、创建class
	gec6818_beep_class = class_create(THIS_MODULE, "beep_class");
	if (gec6818_beep_class == NULL)
	{
		pr_err("%s: beep class create error\n", __func__);
		ret = -EBUSY;
		goto class_create_err;
	}

	// 7、创建device
	gec6818_beep_dev = device_create(gec6818_beep_class, NULL, beep_num, NULL, DEV_NAME); // /dev/beep_dev
	if (gec6818_beep_dev == NULL)
	{
		pr_err("%s: beep device create error\n", __func__);
		ret = -EBUSY;
		goto device_create_err;
	}

	printk("GEC6818 beep device driver init\n");
	return ret;

device_create_err:
	class_destroy(gec6818_beep_class);
class_create_err:
	cdev_del(&beep_dev);
cdev_add_err:
	unregister_chrdev_region(beep_num, 1);
	return ret;
}

static void __exit gec6818_beep_exit(void)
{
	device_destroy(gec6818_beep_class, beep_num); // 销毁device
	class_destroy(gec6818_beep_class);			  // 删除class
	cdev_del(&beep_dev);						  // 删除cdev
	unregister_chrdev_region(beep_num, 1);		  // 释放设备号
	printk("GEC6818 beep device driver exit\n");
}

// module的入口和出口
module_init(gec6818_beep_init); // 安装驱动:#insmod beep_drv.ko --->module_init()--->gec6818_beep_init( )
module_exit(gec6818_beep_exit); // 卸载驱动:#rmmod beep_drv.ko --->module_exit()--->gec6818_beep_exit()

// module的描述, #modinfo *.ko
MODULE_AUTHOR("2264162556@qq.com");
MODULE_DESCRIPTION("GEC6818: BEEP Device Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("V1.0");
