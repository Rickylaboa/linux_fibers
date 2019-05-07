#include <fiber_struct.h>

extern struct fiber_struct* init_fiber(int status,int pid,long index,struct pt_regs regs){
    struct fiber_struct* new_fiber =  kmalloc(sizeof(struct fiber_struct), __GFP_HIGH);
    if(new_fiber==NULL){
        printk(KERN_ERR "%s: error in kmalloc\n",NAME);
        return NULL;   
    }
    new_fiber->status=status;
    new_fiber->pid=pid;
    new_fiber->index=index;
    new_fiber->registers=regs;
    return new_fiber;
}

extern int add_fiber(struct fiber_struct* f){ // TO MODIFY
    unsigned long v=0;
    spin_lock_irqsave(&list_lock,v); 
    if(list_f==NULL){ 
        list_f =  (struct fiber_list*) kmalloc(sizeof(struct fiber_list), __GFP_HIGH);
        if(list_f==NULL){
            printk(KERN_ERR "%s: error in kmalloc\n",NAME);
            spin_unlock_irqrestore(&list_lock,v); 
            return -1;
        }
        list_f->data = f;
        list_f->next = NULL;
    }else{
        struct fiber_list* temp_list=list_f;
        while(temp_list->next!=NULL) temp_list=temp_list->next;
        temp_list->next = (struct fiber_list*) kmalloc(sizeof(struct fiber_list), __GFP_HIGH);
        if(temp_list->next==NULL){
            printk(KERN_ERR "%s: error in kmalloc\n",NAME);
            spin_unlock_irqrestore(&list_lock,v); 
            return -1;   
        }
        temp_list->data->pid = current->parent->pid;
        temp_list->next->data = f;
        temp_list->next->next = NULL;
    }
    spin_unlock_irqrestore(&list_lock,v); 
    return 0;
}


extern int remove_fiber(long index){ // TO MODIFY
    unsigned long v=0;
    int ret=0;
    spin_lock_irqsave(&list_lock,v);
    int element_pid = list_f->data->pid;
    long element_index = list_f->data->index;
    if(element_pid==(current->parent->pid) && element_index ==index)
    {
        struct fiber_list* t = list_f;
        list_f = list_f->next;
        ret=1;
        kfree(t);
    }
    struct fiber_list* temp = list_f;
    while(temp->next!=NULL)
    {
        int element_pid = temp->next->data->pid;
        long element_index = temp->next->data->index;
        if(element_pid==(current->parent->pid) && element_index ==index)
        {
            struct fiber_list* t = temp->next;
            temp->next = temp->next->next;
            ret=1;
            kfree(t);
        }   
        temp=temp->next;
    }
    
    spin_unlock_irqrestore(&list_lock,v); 
    return 0;
}