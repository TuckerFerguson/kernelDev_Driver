/*
* Majority of this code was derived from the examples located in 
* ~/CS453-resources/examples/device-management/linux-device-drivers
*
*/
#include <linux/module.h>
#include <linux/proc_fs.h>	
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/fs.h>     
#include <linux/errno.h>  
#include <linux/version.h>
#include <linux/signal.h>  
#include <linux/sched.h>  
#include <linux/thread_info.h>  
#include <linux/types.h>  
#include <linux/kernel.h> 
#include <linux/init.h>  
#include <linux/uaccess.h>
#include <linux/slab.h>  
#include <linux/random.h>
//#include <stdio.h>
#include "booga.h"        

static int booga_minors = BOOGA_MINORS;
static int booga_major = BOOGA_MAJOR;
module_param(booga_minors, int, 0);
module_param(booga_major, int, 0);
MODULE_AUTHOR("Shane Panter & Tucker Ferguson");
MODULE_LICENSE("GPL v2");

//provided methods
static int booga_open (struct inode *, struct file *);
static int booga_release (struct inode *, struct file *);
static int booga_proc_open(struct inode *inode, struct file *file);
static ssize_t booga_read (struct file *, char *, size_t , loff_t *);
static ssize_t booga_write (struct file *, const char *, size_t , loff_t *);
extern void get_random_bytes(void *buf, int nbytes);

//helper methods
//static int rand(void);
static void update_stats(void);

/*  This is where we define the standard read,write,open and release function */
static struct file_operations booga_driver_operations = {
    .read =       booga_read,
    .write =      booga_write,
    .open =       booga_open,
    .release =    booga_release,
};

static const struct file_operations  proc_booga_driver_operations= {
     .read	= seq_read,
      .release	= single_release,
      .owner	= THIS_MODULE,
      .llseek	= seq_lseek,
      .open	= booga_proc_open,
};

static booga_stats *booga_device_stats;
static struct proc_dir_entry* booga_proc_file;

static int booga_open (struct inode *inode, struct file *filp)
{
    int min = NUM(inode->i_rdev);
	if (min >= booga_minors) return -ENODEV;
	filp->f_op = &booga_driver_operations;

	if (down_interruptible (&booga_device_stats->sem)) {
		return (-ERESTARTSYS);
	}

	switch (min)
    {
        case 0:
            booga_device_stats->booga0++;
            break;
        case 1:
            booga_device_stats->booga1++;
            break;
        case 2:
            booga_device_stats->booga2++;
            break;
        case 3:
            booga_device_stats->booga3++;
            break;
    }

	up(&booga_device_stats->sem);

	try_module_get(THIS_MODULE);
	return 0;
    
}

static int booga_release (struct inode *inode, struct file *filp)
{
   printk(KERN_INFO "booga: Device successfully closed\n");

	if (down_interruptible (&booga_device_stats->sem)) {
		return (-ERESTARTSYS);
	}
	up(&booga_device_stats->sem);
	
	try_module_get(THIS_MODULE);
	return 0;
}

static ssize_t booga_read (struct file *filp, char *buf, size_t count, loff_t *f_pos){
   // int charcount = atoi(argv[2]);
	char *phrase;
	char *buff;
	char randval;
    int read = 0;
	int junk;
	int i = 0;
	int j = 0;
	int k = 0;
	
	if (count == 0) {
		return count;
	}

	buff = (char *) kmalloc(sizeof(char)*count, GFP_KERNEL);
	memset(buff, '\0', count);

	printk(KERN_INFO "Read returned n characters.\n");
    if (down_interruptible(&booga_device_stats->sem)) {
       return (-ERESTARTSYS);
	}
    booga_device_stats->num_read += count;
    up(&booga_device_stats->sem);
    
    get_random_bytes(&randval, 1);
    i = (randval & 0x7F) % 4; 

    switch (i)
    {
    case 0:
        phrase = "booga! booga! ";
        break;
    case 1:
        phrase = "googoo! gaagaa! ";
        break;
    case 2:
        phrase = "neka! maka! ";
        break;
    case 3:
        phrase = "wooga! wooga! ";
        break;
    }

    if (down_interruptible(&booga_device_stats->sem)) {
        return (-ERESTARTSYS);
	}

    switch (i)
    {
    case 0:
        booga_device_stats->string0++;
        break;
    case 1:
        booga_device_stats->string1++;
        break;
    case 2:
        booga_device_stats->string2++;
        break;
    case 3:
        booga_device_stats->string3++;
        break;
    }

    up(&booga_device_stats->sem);

	for (j = 0; j <= count-1; j++) {
		buff[j] = phrase[k % strlen(phrase)];
		k++;
		read += 1;
	}

    junk = copy_to_user(buf, buff, strlen(buff));
    kfree(buff);
    return count;
}

static ssize_t booga_write (struct file *filp, const char *user_space_buffer, size_t count, loff_t *f_pos)
{
    struct inode *inodep;
	int min_num;
	printk(KERN_INFO "Attempting to write to booga device\n");

	if (count == 0) {
		return 0;
	}

	inodep = filp->f_inode;
	min_num = NUM(inodep->i_rdev);
	if (min_num == 3){
		send_sig(SIGTERM, current, 0);
        return 0;
    }

    if (down_interruptible(&booga_device_stats->sem)) {
        return (-ERESTARTSYS);
	}
    booga_device_stats->num_write += count;
    up(&booga_device_stats->sem);
	
	return count;
}




static int __init booga_init(void)
{
    int result;

    result = register_chrdev(booga_major, "booga", &booga_driver_operations);
    if (result < 0) {
        printk(KERN_WARNING "booga: can't get major %d\n",booga_major);
        return result;
    }
    if (booga_major == 0) {
        booga_major = result;
    }
    printk("<1> booga device driver version 1.0: loaded at major number %d\n", booga_major);
    booga_device_stats = (booga_stats *) kmalloc(sizeof(booga_stats),GFP_KERNEL);
    if (!booga_device_stats) {
      result = -ENOMEM;
      goto fail_malloc;
    }
    
 update_stats();
    booga_proc_file = proc_create("driver/booga", 0, NULL, &proc_booga_driver_operations); 
    if (!booga_proc_file)  {
      result = -ENOMEM;
      goto fail_malloc;
    }

    return 0;
    fail_malloc:
        unregister_chrdev(booga_major, "booga");
        remove_proc_entry("driver/booga", NULL);
        return  result;
}


static void __exit booga_exit(void)
{
    remove_proc_entry("driver/booga", NULL);
    kfree(booga_device_stats);
    unregister_chrdev(booga_major, "booga");
}

static void update_stats(void) {
	booga_device_stats->num_read = 0;
	booga_device_stats->num_write = 0;
	booga_device_stats->booga0 = 0;
	booga_device_stats->booga1 = 0;
	booga_device_stats->booga2 = 0;
	booga_device_stats->booga3 = 0;
	booga_device_stats->string0 = 0;
	booga_device_stats->string1 = 0;
	booga_device_stats->string2 = 0;
	booga_device_stats->string3 = 0;
	sema_init(&booga_device_stats->sem, 1);
}

static int booga_proc_show (struct seq_file *m, void *v) {
	seq_printf(m, "bytes read = %ld\n", booga_device_stats->num_read);
	seq_printf(m, "bytes written = %ld\n", booga_device_stats->num_write);
	seq_printf(m, "number of opens:\n");
	seq_printf(m, "  /dev/simple0 = %ld times\n", booga_device_stats->booga0);
	seq_printf(m, "  /dev/simple0 = %ld times\n", booga_device_stats->booga1);
	seq_printf(m, "  /dev/simple0= %ld times\n", booga_device_stats->booga2);
	seq_printf(m, "  /dev/simple3 = %ld times\n", booga_device_stats->booga3);
	seq_printf(m, "string output:\n");
	seq_printf(m, "booga! booga! = %ld times\n", booga_device_stats->string0);
	seq_printf(m, "googoo! gaagaa! = %ld times\n", booga_device_stats->string1);
	seq_printf(m, "neka! maka! = %ld times\n", booga_device_stats->string2);
	seq_printf(m, "wooga! wooga! = %ld times\n", booga_device_stats->string3);

	return 0;
}


static int booga_proc_open(struct inode *inode, struct file *filp)
{
		return single_open(filp, booga_proc_show, NULL);
}

// static int rand()
// {
//     char randVal;
//     get_random_bytes(&randVal, 1);
//     return (randomValue & 0x7F) % 4; 
// }



module_init(booga_init);
module_exit(booga_exit);
