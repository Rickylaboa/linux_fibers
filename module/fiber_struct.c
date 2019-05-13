#include <fiber_struct.h>

static struct fiber_hash ft;
static struct process_hash pt;
static struct thread_hash tt;
static struct kprobe exit_prober;



/*  This function initializes the three hashtables needed
    to store the correct informations about fibers and their
    relationships with processes, threads and their own ids.*/
extern void init_hashtables(void){
    hash_init(ft.fiber_table); // INIT FIBERS HASH TABLE
    hash_init(pt.process_table); // INIT PROCESSES HASH TABLE
    hash_init(tt.thread_table); // INIT THREADS HASH TABLE
}

/*  This function returns 1 if the current thread is a fiber, 0 otherwise.*/
inline int is_a_fiber(void)
{
    int key;
    struct thread_node* curr;
    unsigned long flags;
    key = current->pid;
    spin_lock_irqsave(&(tt.tt_lock), flags); // begin of critical section
    hash_for_each_possible(tt.thread_table, curr, list,key){
        if(curr == NULL) break;
        if(curr->tid == key){
            spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of critical section
            return 1;
        }
    }
    spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of critical section
    return 0;
}

/*  This function returns the fiber id to the current fiber, if
    the current thread is a fiber, otherwise it returns -1.*/
inline long current_fiber(void)
{
    int key;
    struct hlist_node* n;
    struct thread_node* curr;
    unsigned long flags;
    key = current->pid;
    spin_lock_irqsave(&(tt.tt_lock), flags); // begin of critical section
    hash_for_each_possible_safe(tt.thread_table,curr,n,list,key){
        if(curr->tid == key){
            long active_fiber_index = curr->active_fiber_index;
            spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of critical section
            return active_fiber_index;
        }
    }
    spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of critical section
    printk(KERN_ERR "%s: thread %d not found!\n", NAME, key);
    return -1;
}

/*  This function provides a way to retrieve a fresh index
    for a new fibers. It uses an hashmap for each process
    using fibers. The key to access this map is the pid of
    the process and there is a spinlock to access the hashtable. */
inline long get_new_index(void){

    int key;
    long fresh_index;
    int fiber_limit;
    unsigned long flags;
    struct process_node* curr;
    struct process_node* elem;
    key = current->parent->pid; // the key in pt is the pid

    fresh_index = -1;
    fiber_limit = (2<<MAX_FIBERS)-1;

	spin_lock_irqsave(&(pt.pt_lock), flags); // begin of critical section
    hash_for_each_possible(pt.process_table,curr,list,key){
        if(curr == NULL) break;
        if(curr->pid == key){
            printk(KERN_INFO "%s: fiber limit is %d\n", NAME, fiber_limit);
            if(curr->index >= fiber_limit) // check limit of fibers reached!
            {
                fresh_index = -1;
                spin_unlock_irqrestore(&(pt.pt_lock),flags);
                printk(KERN_ERR "%s: critical error, MAX_FIBERS reached: %d!\n", NAME, fiber_limit);
                return fresh_index;
            }
            curr->index = curr->index +1;
            fresh_index = curr->index;
            spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of critical section
            return fresh_index;
        }
    }
    // It means this is the first fiber of the process
    fresh_index = 0;
    elem = kmalloc(sizeof(struct process_node), __GFP_HIGH);
    if(elem == NULL){
        printk(KERN_INFO "%s: error in kmalloc()\n", NAME);
        spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of critical section
        return -1;
    }
    elem->pid = key;
    elem->index = fresh_index;
    hash_add(pt.process_table, &(elem->list), key);
    spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of critical section
    return fresh_index;
}

/*  This function is used to initialized a new fiber. It takes the status,
    the pid of the process, the pid of the thread, the index of the fiber
    to initialize and the pt_regs. It uses kmalloc to allocate memory and 
    fullfills it with the fields of a fiber. */
extern struct fiber_struct* init_fiber(int status, int pid, int thread_running, long index, struct pt_regs regs){

    struct fiber_struct* new_fiber =  kmalloc(sizeof(struct fiber_struct), __GFP_HIGH);
    if(new_fiber == NULL){
        printk(KERN_ERR "%s: error in kmalloc\n", NAME);
        return NULL;   
    }
    new_fiber->status = status;
    new_fiber->pid = pid;
    new_fiber->thread_running = thread_running;
    new_fiber->index = index;
    new_fiber->registers = regs;
    return new_fiber;
}

/*  This function adds a fiber to the fiber hashtable. It uses a spinlock
    to access the table. */
inline long add_fiber(struct fiber_struct *f){

    long long key;
    unsigned long flags;
    struct fiber_node* elem = kmalloc(sizeof(struct fiber_node), __GFP_HIGH);
    if(elem == NULL){
        printk(KERN_INFO "%s: error in kmalloc()\n", NAME);
        return -1;
    }
    elem->data = *f;
    key = (long long) ((long long) f->pid << MAX_FIBERS) + f->index;
	spin_lock_irqsave(&(ft.ft_lock), flags); // begin of critical section
    hash_add_rcu(ft.fiber_table, &(elem->list), key);
    spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of critical section

    return 0;
}

/*  This function adds a thread to the thread hashtable. It uses a spinlock
    to access the table. */
inline int add_thread(int tid,long active_fiber_index){
    
    int key;
    unsigned long flags;
    struct thread_node* elem = kmalloc(sizeof(struct fiber_node), __GFP_HIGH);
    if(elem == NULL){
        printk(KERN_ERR "%s: error in kmalloc()\n", NAME);
        return -1;
    }
    elem->pid = current->parent->pid;
    elem->tid =  tid;
    elem->active_fiber_index = active_fiber_index;
    key = tid;
	spin_lock_irqsave(&(tt.tt_lock), flags); // begin of critical section
    hash_add(tt.thread_table, &(elem->list), key);
    spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of critical section

    return 0;
}

/*  This function sets a new active fiber index in a thread, if it exists
    into the thread hashtable. It uses a spinlock to access the table and
    returns 1 on success, 0 on failure*/
inline int set_thread(int tid,long active_fiber_index){

    int key;
    struct thread_node* curr;
    struct hlist_node* n;
    unsigned long flags;
    key = current->pid;
    spin_lock_irqsave(&(tt.tt_lock), flags); // begin of critical section
    hash_for_each_possible_safe(tt.thread_table, curr, n, list, key){
        if(curr->tid == key){
            curr->active_fiber_index = active_fiber_index;
            spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of critical section
            return 1;
        }
    }
    spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of critical section
    return 0;
}


/*  To do! */
extern void remove_fiber(long index){ // TO MODIFY
    return;
}

/*  This functions retrieves a fiber by index into the
    fiber hashtable. It uses a spinlock to access the table. */
inline struct fiber_struct* get_fiber(long index){

    long long key;
    unsigned long flags;
    struct fiber_node* curr;
    key = (long long) ((long long) current->parent->pid << MAX_FIBERS) + index; 
	spin_lock_irqsave(&(ft.ft_lock), flags); // begin of critical section
    hash_for_each_possible(ft.fiber_table, curr, list, key){
        if(curr->data.index==index && curr->data.pid == current->parent->pid){
            spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of critical section
            return &curr->data;
        }
    }
    spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of critical section
    return NULL;
}

/*  This function frees all the tables used. No clear if it is useful. 
    Debugging and testing purposes for now. */
extern void free_all_tables(void){

    struct fiber_node* curr1;
    struct process_node* curr2;
    struct thread_node* curr3;

    int bkt1, bkt2, bkt3;
    unsigned long flags;
    int i;
    i=0;

    spin_lock_irqsave(&(ft.ft_lock), flags); // begin of cs
    hash_for_each(ft.fiber_table, bkt1, curr1, list){
        printk(KERN_INFO "%s: deleting fiber %ld in process %d\n",NAME,(curr1->data.index),(curr1->data.pid));
        kfree(curr1);
        i++;
    }  
    spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of cs
    printk(KERN_INFO "%s: FT has issued kfree on %d elements \n", NAME, i);
    i=0;


    spin_lock_irqsave(&(pt.pt_lock), flags); // begin of cs
    hash_for_each(pt.process_table, bkt2, curr2, list){
        kfree(curr2);
        i++;
    }  
    spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of cs
    printk(KERN_INFO "%s: PT has issued kfree on %d elements \n", NAME, i);
    i=0;

    spin_lock_irqsave(&(tt.tt_lock), flags); // begin of cs
    hash_for_each(tt.thread_table, bkt3, curr3, list){
        kfree(curr3);
        i++;
    }  
    spin_unlock_irqrestore(&(tt.tt_lock), flags); // end of cs
    printk(KERN_INFO "%s: TT has issued kfree on %d elements \n", NAME, i);
}

extern int register_exit_handler(void){
    int ret;
    exit_prober.symbol_name  = "do_exit";
    exit_prober.pre_handler = (void*) exit_handler;
    exit_prober.post_handler = (void*) null_handler;
    printk(KERN_INFO "process-probe: Registering process kprobe\n");

    ret=register_kprobe(&exit_prober);
    if(ret<0){
        printk(KERN_ERR "process-probe: error in registering probe");
    }
    return 0;
}

extern int unregister_exit_handler(void){
    unregister_kprobe(&exit_prober);
    return 0;
}


int null_handler(void){
    return 0;
}
int exit_handler(void){
    struct fiber_node* curr2;
    struct process_node* curr1;
    struct thread_node* curr3;
    int bkt2, bkt3,pid,d=1;
    unsigned long flags;
    int i,j,k;
    pid = current->pid;
    i=0;
    j=0;
    k=0;
    printk(KERN_INFO "PROCESS PROBE: do exit issued: pid %d!",(current->pid));
    spin_lock_irqsave(&(pt.pt_lock), flags); // begin of allfibers cs
    spin_lock_irqsave(&(ft.ft_lock), flags); // begin of process cs
    spin_lock_irqsave(&(tt.tt_lock), flags); // begin of 
    hash_for_each_possible(pt.process_table, curr1, list, pid){
        printk(KERN_INFO "%s: deleting process %d\n",NAME,(curr1->pid));
        hash_del(&(curr1->list));
        kfree(curr1);
        i++;
    } 
    while(d>0)
    { 
        d=0;
        hash_for_each(ft.fiber_table, bkt2, curr2, list){
            if(curr2->data.pid == pid){
                printk(KERN_INFO "%s: deleting (%d) fiber %ld in process %d (current pid: %d)\n",NAME,bkt2,(curr2->data.index),(curr2->data.pid),(current->pid));
                hash_del(&(curr2->list));
                kfree(curr2);
                j++;
                d++;
            }
        }
    }

    hash_for_each(tt.thread_table, bkt3, curr3, list){
        if(curr3->pid == pid){
            printk(KERN_INFO "%s: deleting thread %d in process %d\n",NAME,(curr3->tid),(curr3->pid));
            hash_del(&(curr3->list));
            kfree(curr3);
            k++;
        }
    }

    spin_unlock_irqrestore(&(tt.tt_lock),flags);
    spin_unlock_irqrestore(&(ft.ft_lock),flags);
    spin_unlock_irqrestore(&(pt.pt_lock),flags);
    if(i>0) printk(KERN_INFO "%s: removed %d processes\n",NAME,i);
    if(j>0) printk(KERN_INFO "%s: removed %d fibers\n",NAME,j);
    if(k>0) printk(KERN_INFO "%s: removed %d threads\n",NAME,k);
    return 0;
}

