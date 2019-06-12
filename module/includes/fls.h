#ifndef FLS_H // Fabio Marra's copyright
#define FLS_H

#include<linux/hashtable.h>
#include"fiber_struct.h"

#define MAX_INDEX 1024


struct fls_data{

    long index; // the index of the fiber local storage data
    void* value; // the pointer to the data
    struct hlist_node list; // the next element
};

long fls_alloc(void);
int fls_free(long index);
void *fls_get_value(long index);
void fls_set_value(long index, void *value);


#endif 

