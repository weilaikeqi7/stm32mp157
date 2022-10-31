#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <asm/io.h>

#include "led_opr.h"

static volatile unsigned int *RCC_PLL4CR;
static volatile unsigned int *RCC_MP_AHB4ENSETR;
static volatile unsigned int *GPIOA_MODER;
static volatile unsigned int *GPIOA_BSRR;
static volatile unsigned int *GPIOG_MODER;
static volatile unsigned int *GPIOG_BSRR;

static int board_demo_led_init(int which)
{
	printk("%s %s line %d, led %d\n", __FILE__, __FUNCTION__, __LINE__, which);
	
	if (!RCC_PLL4CR)
	{
		RCC_PLL4CR = ioremap(0x50000000 + 0x894, 4);
		RCC_MP_AHB4ENSETR = ioremap(0x50000000 + 0xA28, 4);
		GPIOA_MODER = ioremap(0x50002000 + 0x00, 4);
		GPIOA_BSRR = ioremap(0x50002000 + 0x18, 4);
		GPIOG_MODER = ioremap(0x50008000 + 0x00, 4);
		GPIOG_BSRR = ioremap(0x50008000 + 0x18, 4);
	}
	
	if (which == 0)
	{
		*RCC_PLL4CR |= (1<<0);
		while ((*RCC_PLL4CR & (1<<1)) == 0);
		*RCC_MP_AHB4ENSETR |= (1<<0);
		*GPIOA_MODER &= ~(3<<20);
		*GPIOA_MODER |= (1<<20);
	}
	else if (which == 1)
	{
		*RCC_PLL4CR |= (1<<0);
		while ((*RCC_PLL4CR & (1<<1)) == 0);
		*RCC_MP_AHB4ENSETR |= (1<<6);
		*GPIOG_MODER &= ~(3<<16);
		*GPIOA_MODER |= (1<<16);
	}
	
	return 0;
}

static int board_demo_led_ctl(int which, char status)
{
	printk("%s %s line %d, led %d, %s\n", __FILE__, __FUNCTION__, __LINE__, which, status ? "on" : "off");	
	if (which == 0)
	{
		if (status)
		{
			*GPIOA_BSRR = (1<<26);
		}
		else
		{
			*GPIOA_BSRR = (1<<10);
		}
	}
	else if (which == 1)
	{
		if (status)
		{
			*GPIOG_BSRR = (1<<24);
		}
		else
		{
			*GPIOG_BSRR = (1<<8);
		}
	}
	return 0;
}

static struct led_operations board_demo_led_opr = {
	.num = 2,
	.init = board_demo_led_init,
	.ctl = board_demo_led_ctl,
};

struct led_operations *get_board_led_opr(void)
{
	return &board_demo_led_opr;
}
