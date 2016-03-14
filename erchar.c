/**
 * @file erchar.c
 * @author Erik Lux
 * @date 14 March 2016
 * @version 0.1
 * @brief Introductory character driver
 * @see http//:eeerik9.org
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>			// Required for the copy to user function
#define DEVICE_NAME "erchar"		///< The device will appear at /dev/ebbchar using this value
#define CLASS_NAME "er"			///< The device class -- this is a character device driver


static int majorNumber;
static char message[256] = {0};
static short size_of_message;
static struct class* ercharClass = NULL;
static struct device* ercharDevice = NULL;

static int numberOpens;


// The prototype functions for character driver
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = 
{
  .open = dev_open,
  .read = dev_read,
  .write = dev_write,
  .release = dev_release,
};

static int __init erchar_init(void){
  printk(KERN_INFO "ERChar: Initializing the ERChar LKM\n");
  
  // Try to dynamically allocate a major number for the device
  majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
  if (majorNumber < 0) {
    printk(KERN_ALERT "ERChar failed to register a major number\n");
    return majorNumber;
  }
  printk(KERN_INFO "ERChar: registered correctly with major number %d\n", majorNumber);
  
  // Register the device class
  ercharClass = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(ercharClass)){
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_ALERT "Failed to register device class\n");
    return PTR_ERR(ercharClass);
  }
  printk(KERN_INFO "ERChar: device class registered correctly\n");
  
  // Register the device driver
  ercharDevice = device_create(ercharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
  if (IS_ERR(ercharDevice)){
    class_destroy(ercharClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_ALERT "Failed to create the device\n");
  }
  printk(KERN_INFO "ERChar: device class created correctly\n");
  return 0;
}

static void __exit erchar_exit(void){
  device_destroy(ercharClass, MKDEV(majorNumber, 0));
  class_unregister(ercharClass);
  class_destroy(ercharClass);
  unregister_chrdev(majorNumber, DEVICE_NAME);
  printk(KERN_INFO "ERChar: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inodep, struct file* filep){
  numberOpens++;
  printk(KERN_INFO "ERChar: Device has been opened %d time(s)\n", numberOpens);
  return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t* offset){
  int error_count = 0;
  
  error_count = copy_to_user(buffer, message, size_of_message);
  
  if (error_count == 0){
    printk(KERN_INFO "ERChar: Sent %d characters to the user\n", size_of_message);
    return (size_of_message=0); // clear the position to the start and return 0
  } else {
    printk(KERN_INFO "ERChar: Failed to send %d characters to the user\n", error_count);
    return -EFAULT;
  }
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   sprintf(message, "%s(%d letters)", buffer, len);
   size_of_message = strlen(message);
   printk(KERN_INFO "ERChar: Received %d characters from the user\n", len);
   return len;
}

static int dev_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "ERChar: Device successfullly closed\n");
    return 0;
}

module_init(erchar_init);
module_exit(erchar_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Erik LuX");
MODULE_DESCRIPTION("A linux char driver module");
MODULE_VERSION("0.1");
