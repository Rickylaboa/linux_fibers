#include <fls.h>

long fls_alloc(void){
    struct fiber_struct* f = get_fiber(current_fiber());
    struct fls_list *first;
    long index; 
    first=list_first_entry_or_null(&(f->free_fls_indexes->list),struct fls_list,list);
    if(!first){
        index = ++f->max_fls_index;
    }else{
    }

    return 0;
}

void fls_free(long index){
    return;
}
void* fls_get_value(long index){
    return NULL;
}
void fls_set_value(long index, void* value){
    return;
}