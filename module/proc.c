#include<includes/proc.h>
#include<linux/string.h>


static struct file_operations fops = {

  .owner = THIS_MODULE,      //macro for the current module
  .open = NULL,
  .read = NULL,
};

int proc_init_process(int pid){

    int ret;
    struct proc_dir_entry *p;
    char pstr[64];
    struct path p;
    snprintf(pstr, 64, "/proc/%d", pid);
    ret = kern_path(pstr,LOOKUP_FOLLOW,&p);
    if(!ret){
        printk(KERN_ERR "unable to find %s\n",pstr);
        return 0;
    }
    p = proc_create(path, 0666, NULL, &fops);
    if(!p){

        printk(KERN_ERR "proc not created at %s\n",path);
        return 0;
    }

    return 1;
}


int proc_write(struct file *file, const char *buffer, unsigned long count,void *data){
    return 0;
}

int proc_read(struct file *file, const char *buffer, unsigned long count,void *data){
    return 0;
}

int proc_open(struct file *file, const char *buffer, unsigned long count,void *data){
    return 0;
}