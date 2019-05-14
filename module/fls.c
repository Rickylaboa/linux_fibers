#include <fls.h>

long fls_alloc(struct fiber_struct* fiber){
    return 0;
}

void fls_free(struct fiber_struct* fiber,long index){
    return;
}
void* fls_get_value(struct fiber_struct* fiber,long index){
    return NULL;
}
void fls_set_value(struct fiber_struct* fiber,long index){
    return;
}