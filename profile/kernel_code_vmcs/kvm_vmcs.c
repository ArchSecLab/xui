#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kvm_host.h>
#include <linux/kvm.h>
#include <asm/vmx.h>
#include <linux/smp.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "kvm_vmcs_mod"
#define CLASS_NAME "kvm_vmcs"
#define IOCTL_MODIFY_VMCS _IOW('k', 1, int)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("KVM VMCS Modifier Module with ioctl support");
MODULE_VERSION("0.1");

static int major_number;
static struct class *kvm_vmcs_class = NULL;
static struct device *kvm_vmcs_device = NULL;

static int modify_vmcs()
{
    u32 cpu_based_exec_ctrl;

    // Read the current CPU-based VM Execution Controls
    cpu_based_exec_ctrl = __vmx_vmread(CPU_BASED_VM_EXEC_CONTROL);

    // Set the RDTSC exiting bit (bit 12)
    cpu_based_exec_ctrl |= CPU_BASED_RDTSC_EXITING;

    // Write the modified value back to the VMCS
    vmcs_write32(CPU_BASED_VM_EXEC_CONTROL, cpu_based_exec_ctrl);

    return 0;
}
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
    case IOCTL_MODIFY_VMCS:

        modify_vmcs();
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
};

static int __init kvm_vmcs_mod_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0)
    {
        pr_err("Failed to register a major number\n");
        return major_number;
    }

    kvm_vmcs_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(kvm_vmcs_class))
    {
        unregister_chrdev(major_number, DEVICE_NAME);
        pr_err("Failed to register device class\n");
        return PTR_ERR(kvm_vmcs_class);
    }

    kvm_vmcs_device = device_create(kvm_vmcs_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(kvm_vmcs_device))
    {
        class_destroy(kvm_vmcs_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        pr_err("Failed to create the device\n");
        return PTR_ERR(kvm_vmcs_device);
    }

    pr_info("KVM VMCS Modifier Module Loaded: device created with major number %d\n", major_number);

    return 0;
}

static void __exit kvm_vmcs_mod_exit(void)
{
    device_destroy(kvm_vmcs_class, MKDEV(major_number, 0));
    class_unregister(kvm_vmcs_class);
    class_destroy(kvm_vmcs_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("KVM VMCS Modifier Module Unloaded\n");
}

module_init(kvm_vmcs_mod_init);
module_exit(kvm_vmcs_mod_exit);