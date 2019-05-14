#include <includes/fls.h>

/*  This function retrieves the current fiber, checking if the
    list of freed indexes contains a fresh index. If so, this is
    removed and assigned to the new fls data, otherwise
    the max_fls_index is incremented, the data are allocated and the
    fls index is returned to userspace. */
long fls_alloc(void){

    struct fiber_struct* f = get_fiber(current_fiber());
    struct fls_list *first;
    struct fls_data *data;
    long index; 
    first = list_first_entry_or_null(&(f->free_fls_indexes->list), struct fls_list, list);
    data = kmalloc(sizeof(struct fls_data), __GFP_HIGH);
    if(!data){
        printk(KERN_ERR "%s: Error in kmalloc()\n", NAME);
        return -1;
    }
    if(!first){
        index = ++f->max_fls_index;
    }
    else{
        index = first->index;
        list_del(&(first->list));

    }
    data->index = index; 
    data->value = 0; 
    hash_add(f->fls_table, &(data->list), index);
    return index;
}

/*  This function frees an index of the fls, adding this to
    the free list only if it is the max index. Then returns 0
    in case of success. */
int fls_free(long index){

    struct fiber_struct* f = get_fiber(current_fiber());
    struct fls_list *first;
    struct fls_data *data;

    if(f->max_fls_index == index){
        (f->max_fls_index)--;
    }
    else {
        first = kmalloc(sizeof(struct fls_list), __GFP_HIGH);
        if(!first){
            printk(KERN_ERR "%s: Error in kmalloc()\n", NAME);
            return -1;
        }
        first->index = index;
        list_add(&(first->list), &(f->free_fls_indexes->list));
    }
    hash_for_each_possible(f->fls_table, data, list, index){
        if(data->index == index){
            hash_del(&(data->list));
            kfree(data);
        }
    }

    return 0;
}

/*  This function retrieves a data by index, searching it
    in the corresponding hash table. */
void *fls_get_value(long index){

    struct fiber_struct* f = get_fiber(current_fiber());
    struct fls_data *data;
    
    hash_for_each_possible(f->fls_table, data, list, index){
        if(data->index == index){
            return data->value;
        }
    }

    return NULL;
}

/*  This function sets a new value for the passed index,
    searching the current value in the hash table and 
    replacing it with the passed value. */
void fls_set_value(long index, void* value){

    struct fiber_struct* f = get_fiber(current_fiber());
    struct fls_data *data;
    hash_for_each_possible(f->fls_table, data, list, index){
        if(data->index == index){
            data->value = value;
        }
    }
}
