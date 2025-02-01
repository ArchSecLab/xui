#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/printk.h>
#include <linux/mm.h>
#include <linux/pid.h>

#include "uittmonL.h"
#include <asm/uintr.h>
#include <asm/io.h> //virt_to_phys
#define NUM_TESTS 1000

MODULE_LICENSE("GPL");

static int uittmon_mmap(struct file *filp, struct vm_area_struct *vma);
static int uittmon_open(struct inode *uittmon_inode, struct file *uittmon_file);
static int uittmon_release(struct inode *uittmon_inode, struct file *uittmon_file);
static ssize_t uittmon_read(struct file *p_file, char *u_buffer, size_t count, loff_t *ppos);
static ssize_t uittmon_write(struct file *p_file, const char *u_buffer, size_t count, loff_t *ppos);
static long uittmon_ioctl(struct file *file, unsigned ioctl_num, unsigned long ioctl_param);
static long uittmon_ioctl32(struct file *file, unsigned ioctl_num, unsigned long ioctl_param);

dev_t uittmon_dev;
struct cdev *uittmon_cdev;
uint8_t *interrupt_state_ptr = NULL;
struct file_operations uittmon_fops = {
    .owner = THIS_MODULE,
    .read = uittmon_read,
    .unlocked_ioctl = uittmon_ioctl,
    .compat_ioctl = uittmon_ioctl32,
    .write = uittmon_write,
    .open = uittmon_open,
    .release = uittmon_release,
    .mmap = uittmon_mmap,
};

pid_t receiver_pid = 0xfffffff;
struct task_struct *receiver_task = NULL;
struct uintr_upid *receiver_upid = NULL;
struct uintr_upid *receiver_upid_phys = NULL;

static int uittmon_mmap(struct file *filp, struct vm_area_struct *vma)
{
  // Size of memory region to map
  size_t size = vma->vm_end - vma->vm_start;
  unsigned long offset = vma->vm_pgoff << PAGE_SHIFT; // Offset requested by user space process

  // Calculate physical address
  // unsigned long phys_addr = virt_to_phys(driver_buffer);
  unsigned long phys_addr = receiver_upid_phys;
  unsigned long page_offset = offset & (PAGE_SIZE - 1); // Calculate offset within page

  // Adjust physical address to be page aligned
  unsigned long pfn = (phys_addr + page_offset) >> PAGE_SHIFT;

  printk(KERN_INFO "phys_addr = 0x%llx, pfn = 0x%llx\n", phys_addr, pfn);

  if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot))
  {
    printk(KERN_ERR "Failed to remap kernel buffer\n");
    return -EAGAIN;
  }

  // vma->vm_start += phys_addr & (PAGE_SIZE - 1);

  printk(KERN_INFO "mmap succeeded\n", phys_addr, pfn);

  return 0;
}

static int uittmon_open(struct inode *p_inode, struct file *p_file)
{
  return 0;
}

static int uittmon_release(struct inode *p_inode, struct file *p_file)
{
  return 0;
}

u64 t0s[NUM_TESTS], t1s[NUM_TESTS], t2s[NUM_TESTS], t3s[NUM_TESTS], t4s[NUM_TESTS], t5s[NUM_TESTS];

static long uittmon_ioctl32(struct file *file, unsigned ioctl_num, unsigned long ioctl_param)
{
  unsigned long param = (unsigned long)((void *)(ioctl_param));

  uittmon_ioctl(/*file->f_dentry->d_inode,*/ file, ioctl_num, param);
  return 0;
}

void handler(void *arg)
{
  put_user(1, interrupt_state_ptr);
}

// This is the main in/out control function
static long uittmon_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
  printk(KERN_INFO "uittmon: ioctl_num = %u, ioctl_param = %u\n", ioctl_num, ioctl_param);

  if (ioctl_param == 0)
  {
    printk(KERN_ERR "uittmon: ioctl_param is NULL\n");
    return -EINVAL;
  }

  // struct uittmon_io *user_params = (struct uittmon_io*)ioctl_param;
  struct uittmon_io user_params;
  // sendipi as a ioctl
  if (ioctl_num == IOCTL_SENDIPI)
  {
    smp_call_function_single(20, &handler, NULL, 1);
  }
  // register interrupt state as pointer
  else if (ioctl_num == IOCTL_REGISTER_STATE)
  {
    interrupt_state_ptr = (uint8_t *)ioctl_param;
    printk(KERN_INFO "uittmon: interrupt_state_ptr set to 0x%llx\n", interrupt_state_ptr);
  }
  else if (ioctl_num == IOCTL_PROCESS_LIST)
  {
    printk(KERN_INFO "uittmon: ioctl_num = %u, ioctl_param = %lu, user_params = %lu\n", ioctl_num, ioctl_param, user_params);
    if (copy_from_user((void *)(&user_params), (void *)ioctl_param, sizeof(struct uittmon_io)))
    {
      printk(KERN_ERR "uittmon: Failed to copy data from user space\n");
      return -EFAULT;
    }

#ifdef ACCESSPROBLEM // use this if driver cannot access user memory. this occurs rarely

    copy_to_user(user_params, params, sizeof(params));

#endif

    return 0;
  }
  else if (ioctl_num == IOCTL_SET_PID)
  {
    printk(KERN_INFO "Here Before");
    printk(KERN_INFO "current: %llu", current);
    receiver_pid = ioctl_param;
    printk(KERN_INFO "Here after");
    printk(KERN_INFO "upid_ctx: %llu", current->thread.upid_ctx);
    printk(KERN_INFO "Here after after!");
    if (!current->thread.upid_ctx)
    {
      return -EINVAL;
    }
    printk(KERN_INFO "upid: %llu", current->thread.upid_ctx->upid);
    receiver_upid = current->thread.upid_ctx->upid;

    // receiver_upid = virt_to_phys(current->thread.upid_ctx->upid);
    // printk(KERN_INFO "receiver_task: %llu", receiver_task);
    // receiver_upid = receiver_task->thread.upid_ctx->upid;
    receiver_upid_phys = (struct uintr_upid *)virt_to_phys(receiver_upid);
    printk(KERN_INFO "upid_phys: %llu", receiver_upid_phys);
    printk(KERN_DEBUG "uittmon: receiver_upid set to 0x%llx\n", receiver_upid);
    printk(KERN_DEBUG "uittmon: receiver_upid_phys set to 0x%llx\n", receiver_upid_phys);

    return 0;
  }
  else if (ioctl_num == IOCTL_GET_OFFSET)
  {

    u64 page_offset = (u64)receiver_upid_phys & (PAGE_SIZE - 1); // Calculate offset within page
    if (copy_to_user((void *__user)ioctl_param, &page_offset, sizeof page_offset))
      return -EFAULT;

    return 0;
  }
  else
  { // unknown command
    return 1;
  }
  return 1;
}

static ssize_t uittmon_read(struct file *p_file, char *u_buffer, size_t count, loff_t *ppos)
{
  return 0;
}

static ssize_t uittmon_write(struct file *p_file, const char *u_buffer, size_t count, loff_t *ppos)
{
  return 0;
}

static int uittmon_init(void)
{
  uittmon_dev = MKDEV(DEV_MAJOR, DEV_MINOR);
  register_chrdev_region(uittmon_dev, 1, DEV_NAME);

  uittmon_cdev = cdev_alloc();
  uittmon_cdev->owner = THIS_MODULE;
  uittmon_cdev->ops = &uittmon_fops;
  cdev_init(uittmon_cdev, &uittmon_fops);
  cdev_add(uittmon_cdev, uittmon_dev, 1);

  printk("uittmon initialized");

  return 0;
}

static void uittmon_exit(void)
{
  cdev_del(uittmon_cdev);
  unregister_chrdev_region(uittmon_dev, 1);
  printk("uittmon removed");
}

module_init(uittmon_init);
module_exit(uittmon_exit);
