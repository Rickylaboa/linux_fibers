#include<linux/spinlock.h> 
#include "constant.h"

void init_allocations(void);
void add_allocation(void);
void remove_allocation(void);
void print_allocations(void);