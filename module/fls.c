#include<includes/fls.h>

static spinlock_t fls_lock;
static unsigned long fl;

/*  This function retrieves the current fiber, checking if the
    list of freed indexes contains a fresh index. If so, this is
    removed and assigned to the new fls data, otherwise
    the max_fls_index is incremented, the data are allocated and the
    fls index is returned to userspace. */
long fls_alloc(void){

    struct fiber_struct *f = get_fiber(current_fiber());
    struct fls_list *first;
    struct fls_data *data;
    long index; 
    spin_lock_irqsave(&(fls_lock), fl); // begin of allfibers cs
    
    first = list_first_entry_or_null(&(f->free_fls_indexes->list), struct fls_list, list);
    //printk(KERN_INFO "%s: Allocating new index (fib %ld)\n", NAME, f->index); 
    if(unlikely(f->max_fls_index >= MAX_INDEX)){
        printk(KERN_ERR "%s: critical error, max fls reached", NAME);

        return -1;
    }

    data = kzalloc(sizeof(struct fls_data), GFP_KERNEL);
    if(unlikely(!data)){
        printk(KERN_ERR "%s: Error in kzalloc()\n", NAME);
        spin_unlock_irqrestore(&(fls_lock), fl);              //end of critical section

        return -1;
    }
    if(!first){
        index = ++f->max_fls_index;
    }
    else{
        index = first->index;
        list_del(&(first->list));  //remove from free indexes list
        kfree(first);
    }
    data->index = index; 
    data->value = 0; 
    hash_add(f->fls_table, &(data->list), index);

    spin_unlock_irqrestore(&(fls_lock),fl);                   //end of critical section

    return index;
}

/*  This function frees an index of the fls, adding this to
    the free list only if it is the max index. Then returns 0
    in case of success. */
int fls_free(long index){

    struct fiber_struct *f = get_fiber(current_fiber());
    struct fls_list *first;
    struct fls_data *data;

    spin_lock_irqsave(&(fls_lock), fl);                   // begin of all fibers critical section
    if(f->max_fls_index == index){
        (f->max_fls_index)--;
    }
    else {
        first = kzalloc(sizeof(struct fls_list), GFP_KERNEL);
        if(unlikely(!first)){
            printk(KERN_ERR "%s: Error in kzalloc()\n", NAME);
            spin_unlock_irqrestore(&(fls_lock),fl);         //end of critical section

            return -1;
        }
        first->index = index;
        list_add(&(first->list), &(f->free_fls_indexes->list));  //add to the free indexes list
    }
    hash_for_each_possible(f->fls_table, data, list, index){  //remove from the fls hashtable
        if(data->index == index){
            hash_del(&(data->list));
            kfree(data);
        }
    }
    spin_unlock_irqrestore(&(fls_lock),fl);//end of cs

    return 0;
}

/*  This function retrieves a data by index, searching it
    in the corresponding hash table. */
void *fls_get_value(long index){

    struct fiber_struct *f = get_fiber(current_fiber());
    struct fls_data *data;
    
    spin_lock_irqsave(&(fls_lock), fl);              // begin of all fibers critical section
    hash_for_each_possible(f->fls_table, data, list, index){
        if(data->index == index){
            spin_unlock_irqrestore(&(fls_lock),fl);   //end of critical section

            return data->value;
        }
    }
    spin_unlock_irqrestore(&(fls_lock),fl);         //end of critical section

    return NULL;
}

/*  This function sets a new value for the passed index,
    searching the current value in the hash table and 
    replacing it with the passed value. */
void fls_set_value(long index, void *value){

    struct fiber_struct *f = get_fiber(current_fiber());
    struct fls_data *data;

    spin_lock_irqsave(&(fls_lock), fl);                // begin of allfibers critical section
    hash_for_each_possible(f->fls_table, data, list, index){
        if(data->index == index){
            data->value = value;
        }
    }
    spin_unlock_irqrestore(&(fls_lock), fl);          //end of critical section
}
