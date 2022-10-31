#include <linux/module.h>
#include <linux/platform_device.h>

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
#include <linux/of.h>

static int major = 0;
static struct class *led_class;
static struct gpio_desc *led_gpio;

static ssize_t led_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	return 0;
}

static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    int err;
    char status;

    err = copy_from_user(&status, buf, 1);
    gpiod_set_value(led_gpio, status);

    return 1;
}

static int led_drv_open (struct inode *node, struct file *file)
{
    gpiod_direction_output(led_gpio, 0);

    return 0;
}

static int led_drv_close (struct inode *node, struct file *file)
{
	return 0;
}

static struct file_operations led_drv = {
	.owner	 = THIS_MODULE,
	.open    = led_drv_open,
	.read    = led_drv_read,
	.write   = led_drv_write,
	.release = led_drv_close,
};

static int chip_demo_gpio_probe(struct platform_device *pdev)
{
    led_gpio = gpiod_get(&pdev->dev, "led", 0);
    if (IS_ERR(led_gpio))
    {
        dev_err(&pdev->dev, "Failed to get GPIO for led\n");
        return PTR_ERR(led_gpio);
    }

    major = register_chrdev(0, "100ask_led", &led_drv);
    led_class = class_create(THIS_MODULE, "100ask_led_class");
    if (IS_ERR(led_class))
    {
        unregister_chrdev(major, "led");
        gpiod_put(led_gpio);
        return PTR_ERR(led_class);
    }

    device_create(led_class, NULL, MKDEV(major, 0), NULL, "100ask_led%d", 0);

    return 0;
}

static int chip_demo_gpio_remove(struct platform_device *pdev)
{
    device_destroy(led_class, MKDEV(major, 0));
    class_destroy(led_class);
    unregister_chrdev(major, "100ask_led");
    gpiod_put(led_gpio);

    return 0;
}

static const struct of_device_id ask100_leds[] = {
    {.compatible = "100ask,leddrv"},
    {},
};

static struct platform_driver chip_demo_gpio_driver = {
    .probe = chip_demo_gpio_probe,
    .remove = chip_demo_gpio_remove,
    .driver = {
        .name = "100ask_led",
        .of_match_table = ask100_leds,
    },
};

static int __init led_init(void)
{
    int err;

    err = platform_driver_register(&chip_demo_gpio_driver);

    return err;
}

static void __exit led_exit(void)
{
    platform_driver_unregister(&chip_demo_gpio_driver);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
