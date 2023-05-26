/*
#include <stdbool.h>
#include "linked_list.h"
#include "my_mmu.h"
#include "page_table.h"


//**Struct**

typedef struct Process{
    ListItem list;
    int pid;
    PageTable pt;
    bool on_disk; //false phy mem ,true swap mem
}Process;


Process* Process_alloc();
int Process_free(Process* item);
void Process_init(Process* item, int pid);
void Process_print(Process* item);

*/