/**
  * @file   i2c-hello.c
  * @author Erik Lux
  * @date   20 Janueary 2016
*/

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x)  :
#endif

#include <asm/atomic.h>

#include <linux/init.h>         // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>       // Core headers for loading LKMs into the kernel
#include <linux/kernel.h>       // Contains types, macros and functions for the kernel
#include <linux/platform_device.h>
#include <linux/of_device.h>           //defines of_match_ptr
#include <linux/slab.h>         // kzalloc, kfree
#include <linux/uaccess.h>     // copy_to_user, copy_from_user

#include <linux/debugfs.h>

#include <linux/timer.h>    //timer 

#include <linux/workqueue.h>

#include <linux/kthread.h>

#include <linux/devm.h>

#include <linux/mutex.h>

#include <linux/spinlock.h>

#include <linux/err.h>

#include <linux/fs.h>  // macro DEFINE_SIMPLE_ATTRIBUTE

#include <linux/slab.h>

struct mywork_;
typedef struct  {
	int value;
        char * symlink_to;
	
	u32 u32counter;
	u32 u32timeout;
    struct timer_list timer1;
    struct timer_list timer2;
	u32 xyz;
	
	struct mutex work_mutex;
	spinlock_t slock;
	
	struct workqueue_struct *wq;
	struct mywork_ *mywork;
	
	struct task_struct *process;
	
	
	u32 counter2;
	atomic_t timeout2;
	
	u32 counter3;
	atomic_t timeout3;
	
} mystruct_t;

typedef struct mywork_{
  struct delayed_work my_work;
  mystruct_t *mystruct;
} mywork_t;





/*
        
struct attributes {
    int ff;
};

struct attributes *attr;

// ff file contains variable
static ssize_t ff_show(struct device *dev, struct device_attribute *d_attr, char *buf) {
    printk(KERN_ALERT "hello, show ff\n");
    return sprintf(buf, "%d hello\n", attr->ff);
}

static ssize_t ff_store(struct device *dev, struct device_attribute *d_attr, const char *buf, size_t count) {
        printk(KERN_ALERT "hello, store ff\n");
        count = sscanf(buf, "%du", &attr->ff);
        return count;
}

static DEVICE_ATTR(ff,066, ff_show, ff_store);

static struct attribute *attrs[] = {
    &dev_attr_ff.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static struct kobject *mykobj;
*/

static const struct of_device_id hello_of_match[] = {
{
    .compatible = "el,hellomoduledebugfs",
},
{}

};



MODULE_DEVICE_TABLE(of, hello_of_match);


static void mywork_working_func(struct work_struct *work)
{
  mywork_t *mywork= (mywork_t*) work;
  mystruct_t *mystruct = mywork->mystruct;
  unsigned long flags;
  printk(KERN_ALERT "Tick workqueue %d\n", mystruct->counter2);
  
  if (atomic_read(&mystruct->timeout2) == 0) {
    return;
  }
  
  mutex_lock(&mystruct->work_mutex);
  mystruct->counter2++;
  mutex_unlock(&mystruct->work_mutex);
  
  spin_lock_irqsave(&mystruct->slock, flags);
  mystruct->counter3++;
  spin_unlock_irqrestore(&mystruct->slock, flags);
  

  
  queue_delayed_work(mystruct->wq, (struct delayed_work*)mywork, msecs_to_jiffies(atomic_read(&mystruct->timeout2)));
}


static int debugfs_open(struct inode *inode, struct file *file){
     file->private_data = inode->i_private;
     return 0;
}

static ssize_t debugfs_read (struct file *file, char __user *user_buf, size_t length, loff_t *offset) {
    char *locBuf;
    mystruct_t *mystruct = (mystruct_t *) file->private_data;
    if (*offset >=32) {
        return 0;                
    }
    if (*offset + length > 32){
        length = 32 - *offset;
    }
    locBuf = kzalloc(32, GFP_KERNEL);
    sprintf(locBuf, "%d\n", mystruct->xyz);
    if (copy_to_user(user_buf, locBuf + *offset, length)) {
            return -EFAULT;
    }
    *offset +=length;
    kfree(locBuf);
    return length;
}


static ssize_t debugfs_write (struct file *file, const char __user *user_buf, size_t length, loff_t *offset) {
    char *locBuf;
     mystruct_t *mystruct = (mystruct_t *) file->private_data;
    // user_buf is longer than ui32    
    if (length > 10) {
           return -EINVAL;
    } 
    if (*offset >=32) {
            return 0;
    }
    if (*offset + length > 32) {
            length = 32 - *offset;
    }
    locBuf = kzalloc(32, GFP_KERNEL);
    if (copy_from_user(locBuf+ *offset, user_buf, length)){
        return -EFAULT;
    }

    *offset += length;
    sscanf(locBuf, "%du", &mystruct->xyz);
    kfree(locBuf);
    return length;
}




static int counter2_open (struct inode *inode, struct file *file){
     file->private_data = inode->i_private;
     return 0;
}

static ssize_t counter2_read (struct file *file, char __user *user_buf, size_t length, loff_t *offset) {
    char *locBuf;
    mystruct_t *mystruct = (mystruct_t *) file->private_data;
    if (*offset >=32) {
        return 0;                
    }
    if (*offset + length > 32){
        length = 32 - *offset;
    }
    locBuf = kzalloc(32, GFP_KERNEL);
    sprintf(locBuf, "%d\n", mystruct->counter2);
    if (copy_to_user(user_buf, locBuf + *offset, length)) {
            return -EFAULT;
    }
    *offset +=length;
    kfree(locBuf);
    return length;
}

static int timeout2_open(struct inode *inode, struct file *file){
     file->private_data = inode->i_private;
     return 0;
}

static ssize_t timeout2_read (struct file *file, char __user *user_buf, size_t length, loff_t *offset) {
    char *locBuf;
    mystruct_t *mystruct = (mystruct_t *) file->private_data;
    if (*offset >=32) {
        return 0;                
    }
    if (*offset + length > 32){
        length = 32 - *offset;
    }
    locBuf = kzalloc(32, GFP_KERNEL);
    sprintf(locBuf, "%d\n", atomic_read(&mystruct->timeout2));
    if (copy_to_user(user_buf, locBuf + *offset, length)) {
            return -EFAULT;
    }
    *offset +=length;
    kfree(locBuf);
    return length;
}

static ssize_t timeout2_write (struct file *file, const char __user *user_buf, size_t length, loff_t *offset) {
    char *locBuf;
    u32 old_value;
     u32 tmp_read;
    
     mystruct_t *mystruct = (mystruct_t *) file->private_data;
    // user_buf is longer than ui32    
    if (length > 10) {
           return -EINVAL;
    } 
    if (*offset >=32) {
            return 0;
    }
    if (*offset + length > 32) {
            length = 32 - *offset;
    }
    locBuf = kzalloc(32, GFP_KERNEL);
    if (copy_from_user(locBuf+ *offset, user_buf, length)){
        return -EFAULT;
    }

    *offset += length;
    
    old_value = atomic_read(&mystruct->timeout2);
   
    sscanf(locBuf, "%du", &tmp_read);
    atomic_set(&mystruct->timeout2, tmp_read);
    
    
    
 
    if (old_value == 0 && atomic_read(&mystruct->timeout2) != 0){    
      mod_timer(&mystruct->timer2, jiffies + msecs_to_jiffies(atomic_read(&mystruct->timeout2)));
      queue_delayed_work(mystruct->wq, (struct delayed_work*)mystruct->mywork, msecs_to_jiffies(atomic_read(&mystruct->timeout2)));

    }

    kfree(locBuf);
    return length;
}


void timer_func1 (unsigned long data){
      unsigned long flags;
    mystruct_t *mystruct;
    
    mystruct = (mystruct_t*) data;

    
    spin_lock_irqsave(&mystruct->slock, flags);
    mystruct->u32counter++;
    spin_unlock_irqrestore(&mystruct->slock, flags);
    
    mod_timer(&mystruct->timer1, jiffies + msecs_to_jiffies(mystruct->u32timeout));
}

int kthread_working_fct(void *data){
    mystruct_t * mystruct = (mystruct_t*) data;
    unsigned long flags;
    mystruct->counter2++;
   
   while (!kthread_should_stop()){
	if (atomic_read(&mystruct->timeout2) == 0){
	   set_current_state(TASK_INTERRUPTIBLE);
	   schedule();
	}
	mutex_lock(&mystruct->work_mutex);
	mystruct->counter2++;
	mutex_unlock(&mystruct->work_mutex);
	spin_lock_irqsave(&mystruct->slock, flags);
	mystruct->counter3++;
	spin_unlock_irqrestore(&mystruct->slock, flags);
        printk(KERN_ALERT "tick thread: %d\n", mystruct->counter2);
      mod_timer(&mystruct->timer2, jiffies + msecs_to_jiffies(atomic_read(&mystruct->timeout2)));
      
      set_current_state(TASK_INTERRUPTIBLE);
      schedule();
      }
    return 0;
   
}

void timer_func2 (unsigned long data){
    mystruct_t *mystruct;
    mystruct = (mystruct_t*) data;

   wake_up_process(mystruct->process);
  
   
}

  static  const struct file_operations debugfs_file_ops = {
	    .owner = THIS_MODULE,
	    .open = debugfs_open,
	    .read = debugfs_read,
	    .write = debugfs_write,
    };

 static const struct file_operations counter2_file_ops = {
	    .owner = THIS_MODULE,
	    .open = counter2_open,
	    .read = counter2_read,
    };
    
  static  const struct file_operations timeout2_file_ops = {
	    .owner = THIS_MODULE,
	    .open = timeout2_open,
	    .read = timeout2_read,
	    .write = timeout2_write,
    };
    
static int debugfs_u32_set(void *data, u64 val)
{
  mystruct_t *mystruct = (mystruct_t *) data;
  mystruct->counter3 = val;
 return 0;
}
static int debugfs_u32_get(void *data, u64 *val)
{
  mystruct_t *mystruct = (mystruct_t *) data;
  *val =  mystruct->counter3;
  return 0;
}
    
static int debugfs_atomic_t_set(void *data, u64 val)
{
  mystruct_t *mystruct = (mystruct_t *) data;
 atomic_set(&mystruct->timeout3, val);
 return 0;
}
static int debugfs_atomic_t_get(void *data, u64 *val)
{
  mystruct_t *mystruct = (mystruct_t *) data;
 *val = atomic_read(&mystruct->timeout3);
 return 0;
}
  
DEFINE_SIMPLE_ATTRIBUTE(counter3_file_ops, debugfs_u32_get, debugfs_u32_set, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(timeout3_file_ops, debugfs_atomic_t_get, debugfs_atomic_t_set, "%llu\n");
     

static int i2c_hello_probe(struct platform_device *pdev)
{
    mystruct_t *mystruct;
    mywork_t* mywork;
    int err;
    
    struct dentry * directory;
    struct dentry * counter1;
    struct dentry * timeout1;
    struct dentry * timeout;
    struct dentry * symlink;
    struct dentry * counter2;
    struct dentry * timeout2;
    struct dentry * counter3;
    struct dentry * timeout3;
    
    int ret_val;

    char dirname[50];
    
    ret_val = 0;
    
    mystruct = (mystruct_t *) devm_kmalloc(&pdev->dev, sizeof(mystruct_t), GFP_KERNEL);
    if (!mystruct) {
        return -ENOMEM;
    }
    
    mystruct->value = 2016;

    mystruct->symlink_to = "counter1";
  
    mystruct->u32counter = 0; 
    
    mystruct->u32timeout = 1000;
    
    mystruct->xyz = 0;
    
    mystruct->counter2= 0;
    
    atomic_set(&mystruct->timeout2, 10000);
    
    mystruct->counter3= 0;
    
    atomic_set(&mystruct->timeout3, 10000);
 
 
    mutex_init(&mystruct->work_mutex);
    spin_lock_init(&mystruct->slock);
   
    mystruct->wq = create_workqueue("hello_queue");
    if (IS_ERR(mystruct->wq)) {
      printk(KERN_ALERT "Unable to create work queue");
      err = PTR_ERR(mystruct->wq);
      mystruct->wq = NULL;
      return err;
    }
    
    mywork = (mywork_t*) devm_kzalloc(&pdev->dev, sizeof(mywork_t), GFP_KERNEL);
    mystruct->mywork = mywork;
 
    
    
    
    
    dev_set_drvdata(&pdev->dev, mystruct);
 

    
/*
    setup_timer(&mystruct->timer1, timer_func1, (unsigned long) mystruct);
    init_timer(&mystruct->timer1);
    mod_timer(&mystruct->timer1, jiffies +msecs_to_jiffies(mystruct->u32counter*mystruct->u32timeout)); 
 */


     mystruct->process = kthread_create(kthread_working_fct, mystruct, "hello_kthread");
 
    if (IS_ERR(mystruct->process)) {
      printk(KERN_ALERT "Unable to start kernel thread");
      err = PTR_ERR(mystruct->process);
      mystruct->process = NULL;
      return err;
    }
    
 
    setup_timer(&mystruct->timer2, timer_func2, (unsigned long) mystruct);
    init_timer(&mystruct->timer2);
    mod_timer(&mystruct->timer2, jiffies + msecs_to_jiffies(atomic_read(&mystruct->timeout2)));

    

    

    INIT_DELAYED_WORK((struct delayed_work*)mywork, mywork_working_func);
    mywork->mystruct = mystruct;
    queue_delayed_work(mystruct->wq, (struct delayed_work*)mywork, msecs_to_jiffies(atomic_read(&mystruct->timeout2)));

        
    strcpy(dirname, "hello_dbg_");
    strcat(dirname, pdev->name);

    directory = devm_debugfs_create_dir(&pdev->dev, dirname, NULL);
    counter1 = devm_debugfs_create_u32(&pdev->dev, "counter1", 0666, directory, &mystruct->u32counter);
    timeout1 = devm_debugfs_create_u32(&pdev->dev, "timeout1", 0666, directory, &mystruct->u32timeout);
    timeout = devm_debugfs_create_file(&pdev->dev, "my_file", 0666, directory, &debugfs_file_ops); 
    symlink = devm_debugfs_create_symlink(&pdev->dev, "counter1_symlink", directory, mystruct->symlink_to);
    
    counter2 = devm_debugfs_create_file(&pdev->dev, "counter2", 0666, directory, &counter2_file_ops);
    timeout2 = devm_debugfs_create_file(&pdev->dev, "timeout2", 0666, directory, &timeout2_file_ops);
    
    counter3 = devm_debugfs_create_file(&pdev->dev, "counter3", 0666, directory, &counter3_file_ops);
    timeout3 = devm_debugfs_create_file(&pdev->dev, "timeout3", 0666, directory, &timeout3_file_ops);



    
    

  /*     
    mykobj = kobject_create_and_add("mykobject", &(pdev->dev.kobj));
    if (!mykobj) {
        return -ENOMEM;
    }
    
    ret_val = sysfs_create_group (mykobj, &attr_group);
    if (ret_val) {
        kobject_put(mykobj);
    }

*/
     
//    printk(KERN_ALERT "hello,debugfs probe!\n");
    printk(KERN_ALERT "hello, probe! Kobject creationg returned with %d\n", ret_val);

   
    return 0;
}

static int i2c_hello_remove(struct platform_device *pdev)
{
    mystruct_t *mystruct = (mystruct_t *) dev_get_drvdata(&pdev->dev);
    
    printk(KERN_ALERT "hello debugfs, remove, my struct value is %d\n", mystruct->value);

    // not needed, it is only there to free struct beforehand
    // devm_kfree(&pdev->dev, mystruct);

    //del_timer(&mystruct->timer1);
    del_timer(&mystruct->timer2);
    cancel_delayed_work((struct delayed_work*)mystruct->mywork); /* no "new ones" */
    flush_workqueue(mystruct->wq); /* wait till all "old ones" finished */
    destroy_workqueue(mystruct->wq);
    // kobject_put(mykobj);
    kthread_stop(mystruct->process);
    
    mutex_destroy(&mystruct->work_mutex);
    

    return 0;
}

static struct platform_driver hello_driver = {
    .probe=     i2c_hello_probe,
    .remove=    i2c_hello_remove,
    .driver ={
        .name ="hello_module_debugfs",
        .owner = THIS_MODULE,
        .pm =
        NULL,
        .of_match_table = of_match_ptr(hello_of_match),
    },
                        
};

//module_platform_driver(hello_driver)

static int __init
hello_i2c_init_driver(void)
{
    printk(KERN_ALERT "hello, debugfs init\n");
    return platform_driver_register(&hello_driver);
    
}
module_init(hello_i2c_init_driver);

static void __exit
hello_i2c_exit_driver(void)
{
    printk(KERN_ALERT "Hello debugfs exith\n");
    platform_driver_unregister(&hello_driver);
}
module_exit(hello_i2c_exit_driver);

MODULE_AUTHOR("Erik");
MODULE_DESCRIPTION("EL hello bus adapter");
MODULE_LICENSE("GPL");
