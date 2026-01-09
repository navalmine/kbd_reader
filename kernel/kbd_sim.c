#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/kprobes.h>
#include <linux/io.h>

#define BUFFER_SIZE 4096
#define MODULE_NAME "kbd"

static unsigned int interval_ms = 120;
module_param(interval_ms, uint, 0644);
MODULE_PARM_DESC(interval_ms, "Timer interval for simulated scancodes");

static const unsigned char scancodes[] = {
    0x23, 0x12, 0x26, 0x26, 0x18, 0x39,
    0x11, 0x18, 0x13, 0x26, 0x20, 0x39,
    0x02, 0x03, 0x04, 0x1C
};

static size_t seq_idx;
static DEFINE_SPINLOCK(buffer_lock);
static struct timer_list sim_timer;
static DEFINE_KFIFO(kbd_fifo, unsigned char, BUFFER_SIZE);
static DECLARE_WAIT_QUEUE_HEAD(read_wait);
static struct kprobe kp;
static const char *hook_symbol;

static void buffer_push(unsigned char val) {
  unsigned long flags;

  spin_lock_irqsave(&buffer_lock, flags);
  kfifo_in(&kbd_fifo, &val, 1);
  spin_unlock_irqrestore(&buffer_lock, flags);
  wake_up_interruptible(&read_wait);
}

static ssize_t buffer_pop(char __user *out, size_t count) {
  unsigned long flags;
  size_t copied = 0;
  unsigned char tmp[256];

  while (copied < count) {
    unsigned int chunk = min_t(size_t, count - copied, sizeof(tmp));
    unsigned int out_len = 0;

    spin_lock_irqsave(&buffer_lock, flags);
    if (kfifo_is_empty(&kbd_fifo)) {
      spin_unlock_irqrestore(&buffer_lock, flags);
      break;
    }
    out_len = kfifo_out(&kbd_fifo, tmp, chunk);
    spin_unlock_irqrestore(&buffer_lock, flags);

    if (copy_to_user(out + copied, tmp, out_len)) {
      return -EFAULT;
    }

    copied += out_len;
  }

  return copied;
}

//tmp : disabled
static void sim_timer_fn(struct timer_list *t) {
  buffer_push(scancodes[seq_idx]);
  seq_idx = (seq_idx + 1) % ARRAY_SIZE(scancodes);
  mod_timer(&sim_timer, jiffies + msecs_to_jiffies(interval_ms));
}

static inline bool get_arg_data_byte(struct pt_regs *regs, u8 *out)
{
#if defined(CONFIG_X86_64)
    /* x86_64 SysV: arg1=rdi, arg2=rsi */
    *out = (u8)(regs->si & 0xFF);
    return true;

#elif defined(CONFIG_X86_32)
    /*
     * i386 cdecl: args on stack: [sp+4]=arg1, [sp+8]=arg2
     * pt_regs has sp in regs->sp (or esp). This is trickier/fragile with different configs.
     * We'll attempt to read user/kernel stack safely via probe_kernel_read.
     */
    {
        unsigned long sp = regs->sp;
        unsigned long arg2_addr = sp + 8;
        u8 data;
        if (probe_kernel_read(&data, (void *)arg2_addr, sizeof(data)) == 0) {
            *out = data;
            return true;
        }
        return false;
    }

#elif defined(CONFIG_ARM64)
    /* arm64: arg1=x0, arg2=x1 */
    *out = (u8)(regs->regs[1] & 0xFF);
    return true;

#else
    /* Unsupported arch: add your ABI here */
    (void)regs;
    (void)out;
    return false;
#endif
}

//TODO for me: monitor serio port
static int kp_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
    u8 data;
    if (get_arg_data_byte(regs, &data)){
        buffer_push(data);
        pr_info("scancode : %d", data);
    }
    return 0;
}


//TODO for me: use wait_queue
static ssize_t kbd_sim_read(struct file *file, char __user *buf, size_t len, loff_t *ppos) {
  if (kfifo_is_empty(&kbd_fifo)) {
    if (file->f_flags & O_NONBLOCK)
      return -EAGAIN;
  }
  return buffer_pop(buf, len);
}

static const struct file_operations kbd_sim_fops = {
    .owner = THIS_MODULE,
    .read = kbd_sim_read,
    .llseek = noop_llseek,
};

static struct miscdevice kbd_sim_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "kbd",
    .fops = &kbd_sim_fops,
    .mode = 0444,
};

static int try_register_kprobe(const char *sym)
{
    int ret;

    memset(&kp, 0, sizeof(kp));
    kp.symbol_name = sym;
    kp.pre_handler = kp_pre_handler;

    ret = register_kprobe(&kp);
    if (ret == 0) {
        hook_symbol = sym;
        pr_info(MODULE_NAME ": hooked symbol: %s\n", hook_symbol);
    }
    return ret;
}

static int __init kbd_sim_init(void) {
  int ret = misc_register(&kbd_sim_device);
  if (ret) {
    return ret;
  }

  ret = try_register_kprobe("serio_interrupt");
  if (ret != 0) {
      pr_warn(MODULE_NAME ": register kprobe on serio_interrupt failed: %d, trying atkbd_interrupt\n", ret);
      ret = try_register_kprobe("atkbd_interrupt");
      if (ret != 0) {
          pr_err(MODULE_NAME ": register kprobe failed on both symbols (serio_interrupt, atkbd_interrupt): %d\n", ret);
          misc_deregister(&kbd_sim_device);
          return ret;
      }
  }

  //timer_setup(&sim_timer, sim_timer_fn, 0);
  //mod_timer(&sim_timer, jiffies + msecs_to_jiffies(interval_ms));
  pr_info(MODULE_NAME ": simulated scancode device /dev/kbd\n");
  return 0;
}

static void __exit kbd_sim_exit(void) {
  //timer_delete_sync(&sim_timer);
  misc_deregister(&kbd_sim_device);
  unregister_kprobe(&kp);
}

module_init(kbd_sim_init);
module_exit(kbd_sim_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simulated keyboard scancode device for demo purposes");
MODULE_AUTHOR("Codex");
MODULE_AUTHOR("navalmine");
