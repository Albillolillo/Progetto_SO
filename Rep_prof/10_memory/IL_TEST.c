#include <stdio.h>
#include <stdlib.h>
#include "my_mmu.c"

char buffer_phymem[buffer_size_phymem]; //mem. fisica
char buffer_swapmem[buffer_size_swapmem]; //mem. swap

PoolAllocator phy_allocator; //allocatore per mem. fisica
PoolAllocator swap_allocator; //allocatore per mem. swap

ListProcessHead* processes; //lista processi


int main(int argc, char** argv) {
    //inizializzo: lista processi, allocatore mem. fisica, allocatore per mem. swap
    processes=(ListProcessHead*)malloc(sizeof(ListProcessHead));
    List_init(processes);
    for (int i;i<MAX_NUM_PROCS;i++){
        Process *item=Process_alloc();
        Process_init(item,i);
        List_insert(processes,NULL,item);
    }
    //print
    Process* aux=processes->first;
    while(aux){
        Process_print(aux);
        aux=aux->next;
    }
}