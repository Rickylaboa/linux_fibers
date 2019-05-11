#include<fiber_struct.h>
#include<constant.h>

extern long fiber_create(unsigned long ip, unsigned long sp, unsigned long di);
extern long fiber_convert(void);
extern long fiber_switch(long index);
extern long fiber_alloc(int status, struct pt_regs regs);
