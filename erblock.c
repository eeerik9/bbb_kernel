/*
* dbram.c
*
*  Created on: Jul 28, 2014
*      Author: beblavy
*/


#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

static int major_num = 0;
//module_param(major_num, int, 0);
static int logical_block_size = 512;
//module_param(logical_block_size, int, 0);
static int nsectors = 1024;
//module_param(nsectors, int, 0);

#define KERNEL_SECTOR_SIZE 512

static struct request_queue *Queue;


static struct dbram_device {
       unsigned long size;
       spinlock_t lock;
       u8 *data;
       struct gendisk *gd;
} dbram_device;


static void dbram_request(struct request_queue *q) {
       struct request *req;
       unsigned long offset, bytes_to_write;
       req = blk_fetch_request(q);
       while (req != NULL) {
               if (req == NULL || (req->cmd_type != REQ_TYPE_FS)) {
                       printk (KERN_NOTICE "Skip non-CMD request\n");
                       __blk_end_request_all(req, -EIO);
                       continue;
               }
               offset = blk_rq_pos(req) * logical_block_size;
               bytes_to_write = blk_rq_cur_sectors(req) * logical_block_size;

               if ((offset + bytes_to_write) > dbram_device.size) {
                       printk (KERN_NOTICE "dbram:  Write out of mem !!!\n");
                       return;
               }
               if (rq_data_dir(req))
                       memcpy(dbram_device.data + offset, req->buffer, bytes_to_write);
               else
                       memcpy(req->buffer, dbram_device.data + offset, bytes_to_write);
               if ( ! __blk_end_request_cur(req, 0) ) {
                       req = blk_fetch_request(q);
               }
       }
}


int dbram_getgeo(struct block_device * block_device, struct hd_geometry * geo) {

       long size;

       size = dbram_device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
       geo->cylinders = (size & ~0x3f) >> 6;
       geo->heads = 4;
       geo->sectors = 16;
       geo->start = 0;
       return 0;
}


static struct block_device_operations dbram_ops = {
               .owner  = THIS_MODULE,
               .getgeo = dbram_getgeo
};

static int __init dbram_init(void) {

       dbram_device.size = nsectors * logical_block_size;
       spin_lock_init(&dbram_device.lock);
       dbram_device.data = vmalloc(dbram_device.size);
       if (dbram_device.data == NULL)
               return -ENOMEM;

       Queue = blk_init_queue(dbram_request, &dbram_device.lock);
       if (Queue == NULL) {
               vfree(dbram_device.data);
               return -ENOMEM;
       }
       blk_queue_logical_block_size(Queue, logical_block_size);

       major_num = register_blkdev(major_num, "dbram");
       if (major_num < 0) {
               printk(KERN_WARNING "dbram: unable to get major number\n");
               vfree(dbram_device.data);
               return -ENOMEM;
       }

       dbram_device.gd = alloc_disk(16);
       if (!dbram_device.gd) {
               unregister_blkdev(major_num, "dbram");
       }
       dbram_device.gd->major = major_num;
       dbram_device.gd->first_minor = 0;
       dbram_device.gd->fops = &dbram_ops;
       dbram_device.gd->private_data = &dbram_device;
       strcpy(dbram_device.gd->disk_name, "dbram0");
       set_capacity(dbram_device.gd, nsectors);
       dbram_device.gd->queue = Queue;
       add_disk(dbram_device.gd);
       return 0;
}

static void __exit dbram_exit(void)
{
       del_gendisk(dbram_device.gd);
       put_disk(dbram_device.gd);
       unregister_blkdev(major_num, "dbram");
       blk_cleanup_queue(Queue);
       vfree(dbram_device.data);
}

module_init(dbram_init);
module_exit(dbram_exit);

MODULE_AUTHOR("Dusan");
MODULE_DESCRIPTION("DBRam block driver test");
MODULE_LICENSE("GPL");

