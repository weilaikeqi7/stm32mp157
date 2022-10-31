#include "asm/gpio.h"
#include "linux/gpio.h"
#include "linux/gpio/machine.h"
#include "linux/irqreturn.h"
#include "linux/mod_devicetable.h"
#include "linux/of.h"
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
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>

struct gpio_key{
    int gpio;
    struct gpio_desc *gpiod;
    int flag;
    int irq;
};

static irqreturn_t gpio_key_isr(int irq, void *dev_id)
{
    struct gpio_key *gpio_key = dev_id;
    int val;
    val = gpiod_get_value(gpio_key->gpiod);

    printk("key %d %d\n", gpio_key->gpio, val);

    return IRQ_HANDLED;
}

static struct gpio_key *gpio_keys_100ask;

static int gpio_key_probe(struct platform_device *pdev)
{
    int err;
    int count;
    int i;
    struct device_node *node = pdev->dev.of_node;
    enum of_gpio_flags flag;
    unsigned flags = GPIOF_IN;

    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    count = of_gpio_count(node);
    if (!count)
    {
        printk("%s %s line %d, there isn't any gpio available\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    gpio_keys_100ask = kzalloc(sizeof(struct gpio_key) * count, GFP_KERNEL);

    for (i = 0; i < count; i++)
    {
        gpio_keys_100ask[i].gpio = of_get_gpio_flags(node, i, &flag);
        if (gpio_keys_100ask[i].gpio < 0)
        {
            printk("%s %s line %d, of_get_gpio_flags fail\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }

        gpio_keys_100ask[i].gpiod = gpio_to_desc(gpio_keys_100ask[i].gpio);
        gpio_keys_100ask[i].flag = flag & OF_GPIO_ACTIVE_LOW;

        if (flag & OF_GPIO_ACTIVE_LOW)
            flags |= GPIO_ACTIVE_LOW;

        err = devm_gpio_request_one(&pdev->dev, gpio_keys_100ask[i].gpio, flags, NULL);

        gpio_keys_100ask[i].irq = gpio_to_irq(gpio_keys_100ask[i].gpio);
    }

    for (i = 0; i < count; i++)
    {
        err = request_irq(gpio_keys_100ask[i].irq, gpio_key_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "100ask_gpio_key", &gpio_keys_100ask[i]);
    }

    return 0;
}   

static int gpio_key_remove(struct platform_device * pdev)
{
    struct device_node *node = pdev->dev.of_node;
    int count;
    int i = 0;

    count = of_gpio_count(node);
    for (i = 0; i < count; i++)
    {
        free_irq(gpio_keys_100ask[i].irq, &gpio_keys_100ask[i]);
    }

    kfree(gpio_keys_100ask);

    return 0;
}

static const struct of_device_id ask100_keys[] = {
    { .compatible = "100ask,gpio_key" },
    { },
};

static struct platform_driver gpio_keys_driver = {
    .probe = gpio_key_probe,
    .remove = gpio_key_remove,
    .driver = {
        .name = "100ask_gpio_key",
        .of_match_table = ask100_keys,
    },
};

static int __init gpio_key_init(void)
{
    int err;

    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    err = platform_driver_register(&gpio_keys_driver);

    return 0;
}

static void __exit gpio_key_exit(void)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    platform_driver_unregister(&gpio_keys_driver);
}

module_init(gpio_key_init);
module_exit(gpio_key_exit);
MODULE_LICENSE("GPL");

