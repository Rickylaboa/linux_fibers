#include <fiber_struct.h>

static struct fiber_hash ft;
static struct process_hash pt;
static struct thread_hash tt;


/*  This function initializes the three hashtables needed
    to store the correct informations about fibers and their
    relationships with processes, threads and their own ids.*/
extern void init_fiber_set(void){
    hash_init(ft.fiber_table); // INIT FIBERS HASH TABLE
    hash_init(pt.process_table); // INIT PROCESSES HASH TABLE
    hash_init(tt.thread_table); // INIT THREADS HASH TABLE

}

/*  This function returns 1 if the current thread is a fiber, 0 otherwise.*/
extern int is_a_fiber(void)
{
    struct thread_set* curr;
    unsigned long flags;
    int key;
    key = current->pid;
    spin_lock_irqsave(&(tt.tt_lock), flags); // begin of cs
    hash_for_each_possible(tt.thread_table,curr,list,key){
        if(curr==NULL) break;
        if(curr->tid==key){
            spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of cs
            return 1;
        }
    }
    spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of cs
    return 0;
}

/*  This function returns the fiber id to the current fiber, if
    the current thread is a fiber, otherwise it returns -1.*/
extern long current_fiber(void)
{
    struct thread_set* curr;
    unsigned long flags;
    int key;
    key = current->pid;
    spin_lock_irqsave(&(tt.tt_lock), flags); // begin of cs
    hash_for_each_possible(tt.thread_table,curr,list,key){
        if(curr==NULL) break;
        if(curr->tid==key){
            long af = curr->active_fiber_index;
            spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of cs
            return af;
        }
    }
    spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of cs
    return -1;
}

/*  This function provides a way to retrieve a fresh index
    for a new fibers. It uses an hashmap for each process
    using fibers. The key to access this map is the pid of
    the process and there is a spinlock to access the hashtable. */
extern long get_new_index(void){
    int key;
    long fresh_index;
	unsigned long flags;
    struct process_set* curr;
    struct process_set* elem;
    key = current->parent->pid; // the key in pt is the pid

    fresh_index = -1;

	spin_lock_irqsave(&(pt.pt_lock), flags); // begin of cs
    hash_for_each_possible(pt.process_table,curr,list,key){
        if(curr==NULL) break;
        if(curr->pid==key){
            curr->index = curr->index +1;
            fresh_index = curr->index;
            spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of cs
            return fresh_index;
        }
    }
    // It means this is the first fiber of the process
    fresh_index = 0;
    elem = kmalloc(sizeof(struct process_set), __GFP_HIGH);
    if(elem==NULL){
        printk(KERN_INFO "%s: error in kmalloc()\n",NAME);
        spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of cs
        return -1;
    }
    elem->pid = key;
    elem->index = fresh_index;
    hash_add(pt.process_table,&(elem->list),key);
    spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of cs
    return fresh_index;
}

/*  This function is used to initialized a new fiber. It takes the status,
    the pid of the process, the pid of the thread, the index of the fiber
    to initialize and the pt_regs. It uses kmalloc to allocate memory and 
    fullfills it with the fields of a fiber. */
extern struct fiber_struct* init_fiber(int status,int pid,int thread_running,long index,struct pt_regs regs){
    struct fiber_struct* new_fiber =  kmalloc(sizeof(struct fiber_struct), __GFP_HIGH);
    if(new_fiber==NULL){
        printk(KERN_ERR "%s: error in kmalloc\n",NAME);
        return NULL;   
    }
    new_fiber->status=status;
    new_fiber->pid=pid;
    new_fiber->thread_running=thread_running;
    new_fiber->index=index;
    new_fiber->registers=regs;
    return new_fiber;
}

/*  This function adds a fiber to the fiber hashtable. It uses a spinlock
    to access the table. */
extern long add_fiber(struct fiber_struct *f){
    long long key;
    unsigned long flags;
    struct fiber_set* elem = kmalloc(sizeof(struct fiber_set), __GFP_HIGH);
    if(elem==NULL){
        printk(KERN_INFO "%s: error in kmalloc()\n",NAME);
        return -1;
    }
    elem->data =  *f;
    key = (long long) ((long long) f->pid << MAX_FIBERS) + f->index;
	spin_lock_irqsave(&(ft.ft_lock), flags); // begin of cs
    hash_add_rcu(ft.fiber_table,&(elem->list),key);
    spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of cs
    return 0;
}

/*  This function adds a thread to the thread hashtable. It uses a spinlock
    to access the table. */
extern int add_thread(int tid,long active_fiber_index){
    long long key;
    unsigned long flags;
    struct thread_set* elem = kmalloc(sizeof(struct fiber_set), __GFP_HIGH);
    if(elem==NULL){
        printk(KERN_INFO "%s: error in kmalloc()\n",NAME);
        return -1;
    }
    elem->tid =  tid;
    elem->active_fiber_index = active_fiber_index;
    key = tid;
	spin_lock_irqsave(&(tt.tt_lock), flags); // begin of cs
    hash_add_rcu(tt.thread_table,&(elem->list),key);
    spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of cs
    return 0;
}

/*  This function sets a new active fiber index in a thread, if it exists
    into the thread hashtable. It uses a spinlock to access the table and
    returns 1 on success, 0 on failure*/
extern int set_thread(int tid,long active_fiber_index){
    struct thread_set* curr;
    unsigned long flags;
    int key;
    key = current->pid;
    spin_lock_irqsave(&(tt.tt_lock), flags); // begin of cs
    hash_for_each_possible(tt.thread_table,curr,list,key){
        if(curr==NULL) break;
        if(curr->tid==key){
            curr->active_fiber_index = active_fiber_index;
            spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of cs
            return 1;
        }
    }
    spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of cs
    return 0;
}


/*  To do! */
extern void remove_fiber(long index){ // TO MODIFY
    return;
}

/*  This functions retrieves a fiber by index into the
    fiber hashtable. It uses a spinlock to access the table. */
extern struct fiber_struct* get_fiber(long index){

    long long key;
    unsigned long flags;
    struct fiber_set* curr;
    key = (long long) ((long long) current->parent->pid << MAX_FIBERS) + index; 
	spin_lock_irqsave(&(ft.ft_lock), flags); // begin of cs
    hash_for_each_possible(ft.fiber_table,curr,list,key){
        printk(KERN_INFO "%s: Iterating FT %ld\n",NAME,curr->data.index);
        if(curr==NULL) break;
        if(curr->data.index==index && curr->data.pid == current->parent->pid){
            spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of cs
            return &curr->data;
        }
    }
    spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of cs
    return NULL;
}

/*  This function frees all the tables used. No clear if it is usefull. 
    Debugging and testing purposes for now. */
extern void free_all_tables(void){
    struct fiber_set* curr1;
    struct process_set* curr2;
    int bkt1,bkt2;
    unsigned long flags;
    int i;
    i=0;

    spin_lock_irqsave(&(ft.ft_lock), flags); // begin of cs
    hash_for_each(ft.fiber_table,bkt1,curr1,list){
        kfree(curr1);
        i++;
    }  
    spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of cs
    printk(KERN_INFO "%s: FT has issued kfree on %d elements \n",NAME,i);
    i=0;


    spin_lock_irqsave(&(pt.pt_lock), flags); // begin of cs
    hash_for_each(pt.process_table,bkt2,curr2,list){
        kfree(curr2);
        i++;
    }  
    spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of cs
    printk(KERN_INFO "%s: PT has issued kfree on %d elements \n",NAME,i);

}