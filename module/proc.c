#include<includes/proc.h>
#include<linux/string.h>
//#include<linux/namei.h>
//#include<linux/kallsyms.h>


static struct file_operations fops = {

  .owner = THIS_MODULE,      //macro for the current module
  .open = NULL,
  .read = NULL,
};

int proc_init_process(int pid){

    /*int ret;
    struct proc_dir_entry *p;
    struct inode* i;
    char pstr[64];
    struct path pt;
    
    snprintf(pstr, 64, "proc/%d/", pid);
    ret = kern_path(pstr,LOOKUP_FOLLOW,&pt);
    if(!ret){
        printk(KERN_ERR "unable to find %s\n",pstr);
        return 0;
    }
    i = pt.dentry->d_inode;
    p = (struct proc_dir_entry*) get_proc_dir_entry(i);
    p = proc_create(path, 0666, NULL, &fops);
    if(!p){

        printk(KERN_ERR "proc not created at %s\n",path);
        return 0;
    }*/
    //printk(KERN_INFO "proc dir entry is at %p\n",p);
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