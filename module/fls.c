#include <fls.h>

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

    hash_add(f->fls_table, &(data->list), index);

    return index;
}

int fls_free(long index){

    struct fiber_struct* f = get_fiber(current_fiber());
    struct fls_list *first;

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

    return 0;
}
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
void fls_set_value(long index, void* value){

    struct fiber_struct* f = get_fiber(current_fiber());
    struct fls_data *data;

    hash_for_each_possible(f->fls_table, data, list, index){
        if(data->index == index){
            data->value = value;
        }
    }
}