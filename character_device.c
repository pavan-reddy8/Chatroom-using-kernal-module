#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h> 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kfifo.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include<linux/string.h>
#include<linux/ioctl.h>
#define WR_VALUE _IOW('a' , 'a' ,int32_t *)
#define FIFO_SIZE 128
struct kfifo_rec_ptr_1 arr[10];

pid_t pids[10];
int32_t i=0;
spinlock_t mylock;

// static int myopen(struct inode *inode, struct file *file)
// {	
// 	int ret;
// 	char module_buf[126];
// 	char buf[128];
// 	pids[i] = get_current()->tgid;
	
	
// 	printk("myopen called in module proccess %d, %d\n",pids[i],i);
	
// 	ret = kfifo_alloc(&arr[i], FIFO_SIZE, GFP_KERNEL);
//     if (ret) {
//         printk(KERN_ERR "error kfifo_alloc\n");
//         return ret;
//     }
// 	sprintf(module_buf,"you are client",i);
// 	strcpy(module_buf,"Client initiated");
// 	sprintf(module_buf,"Client initiated:%d",i+1);
// 	kfifo_in_spinlocked(&arr[i], module_buf, sizeof(module_buf), &mylock);
// 	ret = kfifo_out_peek(&arr[i], buf, sizeof(module_buf));
//   	printk(KERN_INFO "%.*s\n", ret, buf);
// 	printk("kfifo created for process %d, %d\n",pids[i],i);
//i++;
// 	return 0;
// }

static int myopen(struct inode *inode, struct file *file)
{	

	printk("myopen called in module proccess");
	return 0;
}



static int myclose(struct inode *inodep, struct file *filp)
{	
	printk("myclose called in module\n");
	return 0;
}

static long int my_ioctl(struct file *file, unsigned cmd, unsigned long arg){
	printk("ioctl init");
	switch(cmd){
		
		case WR_VALUE:{
			printk("ioctl init inside");
			if(copy_from_user(&i,(int32_t *) arg ,sizeof(i) ))
				printk("ioctl error");
			printk("process%d",i);

			int ret;
			char module_buf[126];
			char buf[128];
			pids[i-1] = get_current()->tgid;

			ret = kfifo_alloc(&arr[i-1], FIFO_SIZE, GFP_KERNEL);
			if (ret) {
				printk(KERN_ERR "error kfifo_alloc\n");
				return ret;
			}
			
			strcpy(module_buf,"Client initiated");
			sprintf(module_buf,"Client initiated:%d",i);
			kfifo_in_spinlocked(&arr[i-1], module_buf, sizeof(module_buf), &mylock);
			ret = kfifo_out_peek(&arr[i-1], buf, sizeof(module_buf));
			printk(KERN_INFO "%.*s\n", ret, buf);
			printk("kfifo created for process %d, %d\n",pids[i-1],i);

		}
			break; 			
	}
	return 0;
}

static ssize_t mywrite(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	int j;
	int i;
	int ret;
	pid_t cur_pid = get_current()->tgid;
	char buff[128];
	char module_buf[126];
	char temp[128];
	

	printk("mywrite function called in module %d\n",cur_pid);

	

	len = copy_from_user(module_buf, buf, len);
	printk(module_buf);
	//printk("Received this string from user: %s\n", module_buf);

	for(j =0; j < 10; j++ ){
			printk("Pid at  %d is %d" ,j ,pids[j]);
	}
	
	for(j =0; j < 10; j++ ){
		if((int)cur_pid != (int)pids[j]){
			printk("%d",j);
			kfifo_in_spinlocked(&arr[j], module_buf, sizeof(module_buf), &mylock);
			ret = kfifo_out_peek(&arr[j], buff, sizeof(module_buf));
			printk(KERN_INFO "%.*s\n", ret, buff);
			
			
		}
	}

	//kfree(module_buf); // clean up
	//kfree(buff);
	return len; 

}

static ssize_t myread(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	char *module_buf;
	char buff[128];
	int buflen = strlen("Hello World!\n") + 1;
	int ret;
	pid_t cur_pid = get_current()->tgid;
	int j;
	//printk("myread function called in module\n");

	if( len < buflen)
		return -1; // not enough space in user buffer to return string.

	module_buf = (char *)kmalloc(128, GFP_USER);
	if(!module_buf) 
		return -1; //kmalloc failed

	for(j=0;j<10;j++)
		if((int)cur_pid == (int)pids[j])
			break;

	ret = kfifo_out_spinlocked(&arr[j], buff, sizeof(buff), &mylock);
	//buff[ret] = '\0';
	//printk(KERN_INFO "%.*s\n", ret, buff);
	sprintf(module_buf,"%.*s\n",ret,buff);
	len = copy_to_user(buf,module_buf,128);

	kfree(module_buf); // clean up

	return len; 
}

static const struct file_operations myfops = {
    .owner	= THIS_MODULE,
    .read	= myread,
    .write	= mywrite,
    .open	= myopen,
	.unlocked_ioctl = my_ioctl,
    .release	= myclose,
    .llseek 	= no_llseek,

};

struct miscdevice mydevice = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "chatroom",
    .fops = &myfops,
    .mode = S_IRUGO | S_IWUGO,
};

static int __init my_init(void)
{
	printk("my_init called\n");

	// register the character device
	if (misc_register(&mydevice) != 0) {
		printk("device registration failed\n");
		return -1;
	}

	printk("character device registered\n");
	
    spin_lock_init(&mylock);

	return 0;
}



static void __exit my_exit(void)
{
	printk("my_exit called\n");
	int j;
	for(j=0;j<10;j++){
	kfifo_free(&arr[j]);
	}
	misc_deregister(&mydevice);
}


module_init(my_init)
module_exit(my_exit)
MODULE_DESCRIPTION("Miscellaneous character device module\n");
MODULE_AUTHOR("Kartik Gopalan");
MODULE_LICENSE("GPL");
