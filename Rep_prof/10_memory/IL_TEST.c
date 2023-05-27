#include <stdio.h>
#include <stdlib.h>
#include "my_mmu.c"


PoolAllocator* phy_allocator; //allocatore per mem. fisica
char buffer_phymem[BUFFER_SIZE_PHYMEM]; //mem. fisica

PoolAllocator* swap_allocator; //allocatore per mem. swap
char buffer_swapmem[BUFFER_SIZE_SWAPMEM]; //mem. swap



ListProcessHead* processes; //lista processi


int main(int argc, char** argv) {
    //inizializzo: lista processi
    printf("Inizzializzo processi\n");
    processes=(ListProcessHead*)malloc(sizeof(ListProcessHead));
    List_init(processes);
    for (int i;i<MAX_NUM_PROCS;i++){
        Process *item=Process_alloc();
        Process_init(item,i);
        List_insert(processes,NULL,item);
    }
    List_print(processes);
    Process* current=processes->first;

    //inizializzo: allocatore mem. fisica
    printf("\nInizzializzo mem.fisica\n");
    phy_allocator=PoolAllocator_alloc();
    PoolAllocatorResult ret=PoolAllocator_init(phy_allocator,ITEM_SIZE,NUM_ITEMS_PHYMEM,buffer_phymem,BUFFER_SIZE_PHYMEM);
    printf("%s\n",PoolAllocator_strerror(ret));
    PoolAllocator_PrintInfo(phy_allocator);
    
    //inizializzo: allocatore mem. swap
    printf("\nInizzializzo mem.swap\n");
    swap_allocator=PoolAllocator_alloc();
    ret=PoolAllocator_init(swap_allocator,ITEM_SIZE,NUM_ITEMS_SWAPMEM,buffer_swapmem,BUFFER_SIZE_SWAPMEM);
    printf("%s\n",PoolAllocator_strerror(ret));
    PoolAllocator_PrintInfo(swap_allocator);

    //inizializzo MMU
    printf("\nInizzializzo MMU\n");
    MMU* mmu=MMU_create(phy_allocator,swap_allocator,current,processes);
    MMU_print(mmu);


}