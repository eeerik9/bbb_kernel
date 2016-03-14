/**
  * @file   i2c-hello.c
  * @author Erik Lux
  * @date   20 Janueary 2016
*/

#include <linux/init.h>         // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>       // Core headers for loading LKMs into the kernel
#include <linux/kernel.h>       // Contains types, macros and functions for the kernel
#include <linux/platform_device.h>
#include <linux/of_device.h>           //defines of_match_ptr

MODULE_AUTHOR("Erik");
MODULE_DESCRIPTION("EL hello bus adapter");
MODULE_LICENSE("GPL");


static const struct of_device_id hello_of_match1[] = {
{
    .compatible = "el,hello-builtin-module",
},
{}

};
MODULE_DEVICE_TABLE(of, hello_of_match1);

static int hello_remove(struct platform_device *pdev)
{
    printk(KERN_ALERT "hello, builtin remove\n");
    return 0;
}

static int hello_probe(struct platform_device *pdev)
{
    printk(KERN_ALERT "hello, builtin probe\n");
    return 0;
}

static struct platform_driver hello_driver = {
    .probe=     hello_probe,
    .remove=    hello_remove,
    .driver ={
        .name ="hello_module_builtin",
        .owner = THIS_MODULE,
        .pm =
        NULL,
        .of_match_table = of_match_ptr(hello_of_match1),
    },
                        
};

//module_platform_driver(hello_driver)

static int __init
hello_init_driver(void)
{
    printk(KERN_ALERT "hello, builtin init\n");
    return platform_driver_register(&hello_driver);
    
}
module_init(hello_init_driver);

static void __exit
hello_exit_driver(void)
{
    printk(KERN_ALERT "Hello builtin exit\n");
    platform_driver_unregister(&hello_driver);
}
module_exit(hello_exit_driver);


