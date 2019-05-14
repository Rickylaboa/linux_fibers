#ifndef FLS_H // Fabio Marra's copyright
#define FLS_H

#include<linux/hashtable.h>
#include<fiber_struct.h>

#define MAX_INDEX 1024


struct fls_data{
    long index;
    void* value;
    struct hlist_node list;
};

long fls_alloc(struct fiber_struct* fiber);
void fls_free(struct fiber_struct* fiber,long index);
void* fls_get_value(struct fiber_struct* fiber,long index);
void fls_set_value(struct fiber_struct* fiber,long index);


#endif 

