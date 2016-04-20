#include <linux/sched.h>
#include <linux/rcupdate.h>
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <asm/siginfo.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>
#include <linux/string.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dongkyu Lee <ledgku@naver.com>");
MODULE_DESCRIPTION("Kookmin University. System programming project");
MODULE_VERSION("1.0");

#define PROCFS_MAX_SIZE         10
#define PROCFS_TESTLEVEL        "battery_test"
#define PROCFS_NOTIFYPID        "battery_notify"
#define PROCFS_THRESHOLD        "battery_threshold"
#define PROCFS_FILENUM			    3

#define CHR_DEV_NAME			"my_battery"
#define CHR_DEV_MAJOR			240

/* Declaration of variables used in this module */

static int level = 99;
static int test_level = 50;                      //indicates level of battery remain.
static int notify_pid = 1;
static int threshold = 30;

/* End of declaration */

/* Declaration of ancillary variables */

static char procfs_buffer[PROCFS_MAX_SIZE];     
static unsigned long procfs_buffer_size = 0;    //size of receive side buffer
static struct proc_dir_entry *proc_entry[PROCFS_FILENUM];       //indicates procfs entry.

/* End of declaration */

/*
  Send signal to notify_pid
*/
int send_signal(int signo)
{
	int ret;

	struct siginfo info;
	struct task_struct *t;

	memset(&info, 0, sizeof(struct siginfo));

	info.si_signo = signo;
	info.si_code = SI_USER;

	t = pid_task(find_vpid(notify_pid), PIDTYPE_PID);

	if (!t)
	{
		printk("Can not find %d\n", notify_pid);
		return -EFAULT;
	}

	ret = send_sig_info(signo, &info, t);

	if (ret < 0)
	{
		printk("Signal sending error!!\n");
		return ret;
	}

	return 0;
}
/* Device driver */

int my_battery_open(struct inode *inode, struct file *filp)
{
	int number = MINOR(inode->i_rdev);
	printk("my_battery device open: Minor number is %d\n", number);
	return 0;
}

ssize_t my_battery_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
	int ret = 0;
	int flag = 0;

	if (*f_pos < 0)
		*f_pos = 0;

	snprintf(procfs_buffer, 16, "%d\n", test_level);
	procfs_buffer_size = strlen(procfs_buffer);

	if(*f_pos > procfs_buffer_size){
		return -EFAULT;
	}else if(*f_pos == procfs_buffer_size){
		return 0;
	}

	if(procfs_buffer_size - *f_pos > count)
		ret = count;
	else
		ret = procfs_buffer_size - *f_pos;

	flag = copy_to_user(buf, procfs_buffer + (*f_pos), ret);

	if(flag < 0)
		return -EFAULT;

	*f_pos += ret;

  // SIGUSR1 : power saving mode
  // SIGUSR2 : normal mode
	if (test_level < threshold)
		send_signal(SIGUSR1);
	else
		send_signal(SIGUSR2);

	return count;
}

int my_battery_release(struct inode *inode, struct file *filp)
{
	printk("my_battery device release\n");
	return 0;
}

struct file_operations my_battery_fops =
{
  owner: THIS_MODULE,
  read: my_battery_read,
  open: my_battery_open,
  release: my_battery_release
};

/*
   Implementation of procfs write function
 */
static int test_level_write( struct file *filp, const char *user_space_buffer, unsigned long len, loff_t *off )
{

	int status = 0;
	int requested;

	procfs_buffer_size = len;

	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}

	/* write data to the buffer */
	if ( copy_from_user(procfs_buffer, user_space_buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}

	status  = kstrtoint(procfs_buffer, 10, &requested);
	if(status < 0)
	{
		printk(KERN_INFO "Error while called kstrtoint(...)\n");
		return -ENOMEM;
	}
	// validate level value.
	if(requested<= 0 || requested >= 100){
		printk(KERN_INFO "Invalid battery level.\n");
		return -ENOMEM;
	}
	// accept value.
	test_level = requested;

	return procfs_buffer_size;
}

static int threshold_write( struct file *filp, const char *user_space_buffer, unsigned long len, loff_t *off )
{

	int status = 0;
	int requested;

	procfs_buffer_size = len;

	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}

	/* write data to the buffer */
	if ( copy_from_user(procfs_buffer, user_space_buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}

	status  = kstrtoint(procfs_buffer, 10, &requested);
	if(status < 0)
	{
		printk(KERN_INFO "Error while called kstrtoint(...)\n");
		return -ENOMEM;
	}
	// validate level value.
	if(requested< 0 || requested > 100){
		printk(KERN_INFO "Invalid battery level.\n");
		return -ENOMEM;
	}
	// accept value.
	threshold = requested;

	return procfs_buffer_size;
}

static int notify_pid_write( struct file *filp, const char *user_space_buffer, unsigned long len, loff_t *off )
{

	int status = 0;
	int requested;

	procfs_buffer_size = len;

	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}

	/* write data to the buffer */
	if ( copy_from_user(procfs_buffer, user_space_buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}

	status  = kstrtoint(procfs_buffer, 10, &requested);
	if(status < 0)
	{
		printk(KERN_INFO "Error while called kstrtoint(...)\n");
		return -ENOMEM;
	}

	// accept value.
	notify_pid = requested;

	return procfs_buffer_size;

}

/*
   Implementation of procfs read function
*/
static int test_level_read( struct file *filp, char *user_space_buffer, size_t count, loff_t *off )
{
	int ret = 0;
	int flag = 0;

	if(*off < 0) *off = 0;

	snprintf(procfs_buffer, 16, "%d\n", test_level);
	procfs_buffer_size = strlen(procfs_buffer);

	if(*off > procfs_buffer_size){
		return -EFAULT;
	}else if(*off == procfs_buffer_size){
		return 0;
	}

	if(procfs_buffer_size - *off > count)
		ret = count;
	else
		ret = procfs_buffer_size - *off;

	flag = copy_to_user(user_space_buffer, procfs_buffer + (*off), ret);

	if(flag < 0)
		return -EFAULT;

	*off += ret;

	return ret;

}

static int threshold_read( struct file *filp, char *user_space_buffer, size_t count, loff_t *off )
{
	int ret = 0;
	int flag = 0;

	if(*off < 0) *off = 0;

	snprintf(procfs_buffer, 16, "%d\n", threshold);
	procfs_buffer_size = strlen(procfs_buffer);

	if(*off > procfs_buffer_size){
		return -EFAULT;
	}else if(*off == procfs_buffer_size){
		return 0;
	}

	if(procfs_buffer_size - *off > count)
		ret = count;
	else
		ret = procfs_buffer_size - *off;

	flag = copy_to_user(user_space_buffer, procfs_buffer + (*off), ret);

	if(flag < 0)
		return -EFAULT;

	*off += ret;

	return ret;

}

static int notify_pid_read( struct file *filp, char *user_space_buffer, size_t count, loff_t *off )
{
	int ret = 0;
	int flag = 0;

	if(*off < 0) *off = 0;

	snprintf(procfs_buffer, 16, "%d\n", notify_pid);
	procfs_buffer_size = strlen(procfs_buffer);

	if(*off > procfs_buffer_size){
		return -EFAULT;
	}else if(*off == procfs_buffer_size){
		return 0;
	}

	if(procfs_buffer_size - *off > count)
		ret = count;
	else
		ret = procfs_buffer_size - *off;

	flag = copy_to_user(user_space_buffer, procfs_buffer + (*off), ret);

	if(flag < 0)
		return -EFAULT;

	*off += ret;

	return ret;

}
/*
   Configuration of file_operations

   This structure indicate functions when read or write operation occured.
*/
static const struct file_operations test_level_fops = {
	.write = test_level_write,
	.read = test_level_read
};

static const struct file_operations threshold_fops = {
	.write = threshold_write,
	.read = threshold_read
};

static const struct file_operations notify_pid_fops = {
	.write = notify_pid_write,
	.read = notify_pid_read
};

/*
   This function will be called on initialization of  kernel module
*/
int init_module(void)
{

	int i = 0;
	int reg;

	reg = register_chrdev(CHR_DEV_MAJOR, CHR_DEV_NAME, &my_battery_fops);

	if (reg < 0)
	{
		printk("Major number:%d is using\n", reg);
		return reg;
	}

	proc_entry[0] = proc_create(PROCFS_TESTLEVEL, 0666, NULL, &test_level_fops);
	proc_entry[1] = proc_create(PROCFS_THRESHOLD, 0666, NULL, &threshold_fops);
	proc_entry[2] = proc_create(PROCFS_NOTIFYPID, 0666, NULL, &notify_pid_fops);

	for (; i<PROCFS_FILENUM; ++i)
	{
		if (proc_entry[i] == NULL)
		{
			return -ENOMEM;
		}
	}

	printk(KERN_ALERT "battery_module initialization success!!");

	return reg;

}

/*
   This function will be called on cleaning up of kernel module
*/
void cleanup_module(void)
{
	remove_proc_entry(PROCFS_TESTLEVEL, NULL);
	remove_proc_entry(PROCFS_THRESHOLD, NULL);
	remove_proc_entry(PROCFS_NOTIFYPID, NULL);

	unregister_chrdev(CHR_DEV_MAJOR, CHR_DEV_NAME);

	printk(KERN_ALERT "battery_module cleanup success!!");
}
