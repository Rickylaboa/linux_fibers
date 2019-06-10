#ifndef FIBER_METHODS_H 
#define FIBER_METHODS_H

#include"fiber_struct.h"
#include"constant.h"
#include<includes/proc.h>
#include<linux/proc_fs.h>

extern long fiber_create(unsigned long ip, unsigned long sp);
extern long fiber_convert(void);
extern long fiber_switch(long index);
extern long fiber_alloc(int status, struct pt_regs regs);

#endif 