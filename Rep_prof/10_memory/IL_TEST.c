#include <stdio.h>
#include <stdlib.h>
#include "my_mmu.c"


PoolAllocator* phy_allocator; //allocatore per mem. fisica
char buffer_phymem[BUFFER_SIZE_PHYMEM]; //mem. fisica
void* phy_blocks[NUM_ITEMS_PHYMEM];//lista blocchi di mem. fisica allocati

PoolAllocator* swap_allocator; //allocatore per mem. swap
char buffer_swapmem[BUFFER_SIZE_SWAPMEM]; //mem. swap
void* swap_blocks[NUM_ITEMS_SWAPMEM]; //lista blocchi di mem. swap allocati


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

    
    //test allocatori
/*
    //alloco e libero tutto 
    for (int i=0; i<NUM_ITEMS_PHYMEM; ++i){
        int frame_num=phy_allocator->first_idx;
        FrameItem* frame=(FrameItem*)PoolAllocator_getBlock(phy_allocator,false);
        phy_blocks[i]=frame;
        FrameEntry_init(frame,mmu->curr_proc->pid,frame_num);
        printf("allocation %d, block %p, size%d, pid%d, frame_num%d, buffer_size%d \n", i, frame, phy_allocator->size, frame->pid, frame->frame_num, sizeof(frame->info));  
    }

    for (int i=0; i<NUM_ITEMS_PHYMEM; ++i){
        FrameEntry_print(phy_blocks[i]);
    }


     for (int i=0; i<NUM_ITEMS_PHYMEM; ++i){
    if (phy_blocks[i]){
      printf("releasing... idx: %d, block %p, free %d, owner:%d ... ",
	     i, phy_blocks[i], phy_allocator->size,phy_blocks[i]->pid);
      PoolAllocatorResult release_result=PoolAllocator_releaseBlock(phy_allocator,phy_blocks[i]);
      printf("%s\n", PoolAllocator_strerror(release_result));
    }
    }

    for (int i=0; i<NUM_ITEMS_PHYMEM; ++i){
        FrameEntry_print(phy_blocks[i]);
    }
 
    //alloco 1 libero 1
    for (int i=0; i<NUM_ITEMS_PHYMEM; ++i){
        int frame_num=phy_allocator->first_idx;
        FrameItem* frame=(FrameItem*)PoolAllocator_getBlock(phy_allocator,false);
        phy_blocks[i]=frame;
        FrameEntry_init(frame,mmu->curr_proc->pid,frame_num);
        printf("allocation %d, block %p, size%d, pid%d, frame_num%d, buffer_size%d \n", i, frame, phy_allocator->size, frame->pid, frame->frame_num, sizeof(frame->info));
        FrameEntry_print(phy_blocks[i]);
        printf("releasing... idx: %d, block %p, free %d, owner:%d ... ",
	     i,phy_blocks[i], phy_allocator->size,phy_blocks[i]->pid);
      PoolAllocatorResult release_result=PoolAllocator_releaseBlock(phy_allocator,phy_blocks[i]);
      printf("%s\n", PoolAllocator_strerror(release_result));  
      FrameEntry_print(phy_blocks[i]);
    } 
    */
  
    //alloco pagetable
    for (int i=0; i<NUM_ITEMS_PHYMEM; ++i){
        if(i%2==0){
            FrameItem* frame=FrameEntry_create(mmu,phy_blocks);
            printf("allocation: %d, block: %p, size: %d,next_frame_num: %d, buffer_size: %d, pt_size: %d\n", i, frame, phy_allocator->size,phy_allocator->first_idx,phy_allocator->buffer_size,sizeof(frame->info)); 
            printf("releasing... idx: %d, block %p, free %d, owner: ... ",i,phy_blocks[frame->frame_num], phy_allocator->size);
            PoolAllocatorResult release_result=PoolAllocator_releaseBlock(phy_allocator,phy_blocks[frame->frame_num],false);
            printf("%s\n", PoolAllocator_strerror(release_result));
        }else{
            PageTable* pagetable=PageTable_create(mmu,phy_blocks);
            printf("allocation: %d, block: %p, size: %d,next_frame_num: %d, buffer_size: %d, pt_size: %d\n", i, pagetable, phy_allocator->size,phy_allocator->first_idx,phy_allocator->buffer_size,sizeof(pagetable->pe));
            printf("releasing... idx: %d, block %p, free %d, owner: ... ",i,phy_blocks[pagetable->phymem_addr.frame_index], phy_allocator->size);
            PoolAllocatorResult release_result=PoolAllocator_releaseBlock(phy_allocator,phy_blocks[pagetable->phymem_addr.frame_index],true);
            printf("\n%s", PoolAllocator_strerror(release_result)); 
        }
    }
}