#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/uaccess.h>

#define BUFFER_SIZE 4096

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

static void buffer_push(unsigned char val) {
  unsigned long flags;

  spin_lock_irqsave(&buffer_lock, flags);
  kfifo_in(&kbd_fifo, &val, 1);
  spin_unlock_irqrestore(&buffer_lock, flags);
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

static void sim_timer_fn(struct timer_list *t) {
  buffer_push(scancodes[seq_idx]);
  seq_idx = (seq_idx + 1) % ARRAY_SIZE(scancodes);
  mod_timer(&sim_timer, jiffies + msecs_to_jiffies(interval_ms));
}

static ssize_t kbd_sim_read(struct file *file, char __user *buf, size_t len, loff_t *ppos) {
  ssize_t copied = buffer_pop(buf, len);
  if (copied == 0) {
    if (file->f_flags & O_NONBLOCK) {
      return -EAGAIN;
    }
  }
  return copied;
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

static int __init kbd_sim_init(void) {
  int ret = misc_register(&kbd_sim_device);
  if (ret) {
    return ret;
  }

  timer_setup(&sim_timer, sim_timer_fn, 0);
  mod_timer(&sim_timer, jiffies + msecs_to_jiffies(interval_ms));
  pr_info("kbd_sim: simulated scancode device /dev/kbd\n");
  return 0;
}

static void __exit kbd_sim_exit(void) {
  timer_delete_sync(&sim_timer);
  misc_deregister(&kbd_sim_device);
}

module_init(kbd_sim_init);
module_exit(kbd_sim_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simulated keyboard scancode device for demo purposes");
MODULE_AUTHOR("Codex");
MODULE_AUTHOR("navalmine");
