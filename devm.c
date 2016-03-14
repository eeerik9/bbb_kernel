#include <asm/atomic.h>
#include <linux/device.h>
#include <linux/export.h>
#include <linux/debugfs.h>

static void devm_kobject_release (struct device *dev, void *res){
    kobject_put(*(struct kobject **)res);
}

struct kobject *devm_kobject_create_and_add (struct device *dev, const char *name, struct kobject *parent){
    struct kobject **ptr, *kobj;

    ptr = devres_alloc (devm_kobject_release, sizeof(*ptr), GFP_KERNEL);
    if (!ptr){
            return ERR_PTR(-ENOMEM);
    }
    kobj = kobject_create_and_add("mykobject", &dev->kobj);
    if (kobj) {
            *ptr = kobj;
            devres_add(dev, ptr);
    } else {
            devres_free(ptr);
    }

    return kobj;
}
EXPORT_SYMBOL(devm_kobject_create_and_add);

int devm_kobject_match (struct device *dev, void *res, void *data) {
        struct kobject **tmp = res;
        if (!tmp || !*tmp) {
                WARN_ON(!tmp||!*tmp);
                return 0;
        }
        return *tmp == data;
};
EXPORT_SYMBOL(devm_kobject_match);


void devm_kobject_put (struct device *dev, struct kobject *kobj){
        int retval;
        retval = devres_release(dev, devm_kobject_release, devm_kobject_match, kobj);

        WARN_ON(retval);
}
EXPORT_SYMBOL(devm_kobject_put);

void devm_debugfs_release(struct device *dev, void *dentry) {
    debugfs_remove(*(struct dentry **)dentry);
}
EXPORT_SYMBOL(devm_debugfs_release);

struct dentry *devm_debugfs_create_dir (struct device *dev, const char *name, struct dentry *parent) {
        struct dentry **ptr, *tmp_entry;

        ptr = devres_alloc(devm_debugfs_release, sizeof(*ptr), GFP_KERNEL);
        if (!ptr) {
                return ERR_PTR(-ENOMEM);

        }

        tmp_entry = devres_alloc(devm_debugfs_release, sizeof(*ptr), GFP_KERNEL);
        if (!ptr){
                return ERR_PTR(-ENOMEM);
        }

        tmp_entry = debugfs_create_dir(name, parent);
        if (tmp_entry){
                *ptr = tmp_entry;
                devres_add(dev, ptr);
        } else {
                devres_free(ptr);
        }

        return tmp_entry;
}
EXPORT_SYMBOL(devm_debugfs_create_dir);

struct dentry *devm_debugfs_create_u32(struct device * dev, const char *name, umode_t mode, struct dentry *parent, u32 *value) {
        struct dentry  **ptr, *tmp_entry;

        ptr = devres_alloc(devm_debugfs_release, sizeof(*ptr), GFP_KERNEL);
        if (!ptr) {
                return ERR_PTR(-ENOMEM);

        }

        tmp_entry = debugfs_create_u32(name, mode, parent, value);
        if (tmp_entry) {
                *ptr = tmp_entry;
                devres_add(dev, ptr);
        } else {
                devres_free(ptr);
        }

        return tmp_entry;
}
EXPORT_SYMBOL(devm_debugfs_create_u32);

struct dentry *devm_debugfs_create_atomic_t(struct device * dev, const char *name, umode_t mode, struct dentry *parent, atomic_t *value) {
        struct dentry  **ptr, *tmp_entry;

        ptr = devres_alloc(devm_debugfs_release, sizeof(*ptr), GFP_KERNEL);
        if (!ptr) {
                return ERR_PTR(-ENOMEM);

        }

        tmp_entry = debugfs_create_atomic_t(name, mode, parent, value);
        if (tmp_entry) {
                *ptr = tmp_entry;
                devres_add(dev, ptr);
        } else {
                devres_free(ptr);
        }

        return tmp_entry;
}
EXPORT_SYMBOL(devm_debugfs_create_atomic_t);

struct dentry *devm_debugfs_create_file(struct device * dev, const char *name, umode_t mode, struct dentry *parent, const struct file_operations *debugs_file_ops) {
        struct dentry  **ptr, *tmp_entry;

        ptr = devres_alloc(devm_debugfs_release, sizeof(*ptr), GFP_KERNEL);
        if (!ptr) {
                return ERR_PTR(-ENOMEM);

        }
       	
        tmp_entry = debugfs_create_file(name, mode, parent,dev_get_drvdata(dev), debugs_file_ops);
        if (tmp_entry) {
                *ptr = tmp_entry;
                devres_add(dev, ptr);
        } else {
                devres_free(ptr);
        }

        return tmp_entry;
}
EXPORT_SYMBOL(devm_debugfs_create_file);

struct dentry *devm_debugfs_create_symlink(struct device * dev, const char *name, struct dentry *parent, const char * symlink_to) {
        struct dentry  **ptr, *tmp_entry;

        ptr = devres_alloc(devm_debugfs_release, sizeof(*ptr), GFP_KERNEL);
        if (!ptr) {
                return ERR_PTR(-ENOMEM);

        }
        tmp_entry = debugfs_create_symlink(name, parent, symlink_to);
        if (tmp_entry) {
                *ptr = tmp_entry;
                devres_add(dev, ptr);
        } else {
                devres_free(ptr);
        }

        return tmp_entry;
}
EXPORT_SYMBOL(devm_debugfs_create_symlink);