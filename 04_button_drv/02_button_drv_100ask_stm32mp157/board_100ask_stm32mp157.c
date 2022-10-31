#include <linux/module.h>

#include <linux/fs.h>
#include <linux/io.h>
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

#include "button_drv.h"

struct stm32mp157_gpio
{
    volatile unsigned int MODER;
    volatile unsigned int OTYPER;
    volatile unsigned int OSPEEDR;
    volatile unsigned int PUPDR;
    volatile unsigned int IDR;
    volatile unsigned int ODR;
    volatile unsigned int BSRR;
    volatile unsigned int LCKR;
    volatile unsigned int AFR[2];
};

static volatile unsigned int *RCC_PLL4CR;

static volatile unsigned int *RCC_MP_AHB4ENSETR;

static struct stm32mp157_gpio *gpiog;

static void board_xxx_button_init_gpio (int which)
{
    if (!RCC_PLL4CR)
    {
        RCC_PLL4CR = ioremap(0x50000000 + 0x894, 4);
        RCC_MP_AHB4ENSETR = ioremap(0x50000000 + 0xA28, 4);
        gpiog = ioremap(0x50008000, sizeof(struct stm32mp157_gpio));
    }

    if (0 == which)
    {
        *RCC_PLL4CR |= (1<<0);
        while ((*RCC_PLL4CR & (1<<1)) == 0);

        *RCC_MP_AHB4ENSETR |= (1<<6);

        gpiog->MODER &= ~(3<<6);
    }
    else if (1 == which)
    {
        *RCC_PLL4CR |= (1<<0);
        while ((*RCC_PLL4CR & (1<<1)) == 0);

        *RCC_MP_AHB4ENSETR |= (1<<6);

        gpiog->MODER &= ~(3<<4);
    }
    else
    {
    }
}

static int board_xxx_button_read_gpio (int which)
{
    if (0 == which)
    {
        return (gpiog->IDR & (1<<3) ? 1 : 0);
    }
    else if (1 == which)
    {
        return (gpiog->IDR & (1<<2) ? 1 : 0);
    }
    else
    {
    }
    return 1;
}

static struct button_operations my_buttons_ops = {
    .count = 2,
    .init = board_xxx_button_init_gpio,
    .read = board_xxx_button_read_gpio,
};

int board_xxx_button_init(void)
{
    register_button_operations(&my_buttons_ops);
    return 0;
}

void board_xxx_button_exit(void)
{
    unregister_button_operations();
}

module_init(board_xxx_button_init);
module_exit(board_xxx_button_exit);
MODULE_LICENSE("GPL");
