#include <stdio.h>
#include <stdlib.h>
#include "my_mmu.c"


char buffer_phymem[BUFFER_SIZE_PHYMEM]; //mem. fisica
char buffer_swapmem[BUFFER_SIZE_SWAPMEM]; //mem. swap


int main(int argc, char** argv) {
    
    //inizializzo MMU
    printf("\nInizzializzo MMU\n");
    MMU* mmu=MMU_create(buffer_phymem,buffer_swapmem);
    MMU_print(mmu);
    
    
    for (int i=1;i<MAX_NUM_PROCS;i++){
        Process *item=Process_alloc();
        Process_init(item,i,mmu);
        
    }

    MMU_print(mmu);
    
    PageTable_print(mmu->curr_proc->pt);

    MMU_process_update(mmu);
    PageTable_print(mmu->curr_proc->pt);

    LogicalAddress L_A;
    for (int i=0; i<80; ++i){
        
        L_A.offset=(i*2)%PENTRY_NUM;
        L_A.pt_index=i;
        char c=i%FRAME_NUM;
        MMU_writeByte(mmu,L_A,c);
        
        char* read_byte=MMU_readByte(mmu,L_A);
        if(read_byte){
            printf(": %c\n",*read_byte);
        }
    }
    
    MMU_process_update(mmu);
    
    for (int i=0; i<170; ++i){
        
        L_A.offset=i;
        L_A.pt_index=(i)%PENTRY_NUM;
        char c=i%FRAME_NUM;
        MMU_writeByte(mmu,L_A,c);
        if(i%2==0){
            char* read_byte=MMU_readByte(mmu,L_A);
            if(read_byte){
                printf(": %c\n",*read_byte);
            }
        }else{
            L_A.offset=i+1;
            MMU_writeByte(mmu,L_A,c);
        }
    }
     PageTable_print(mmu->curr_proc->pt);
    
 

//test convertitore LA->PA
/*
    LogicalAddress L_A;
    L_A.offset=0;
    L_A.pt_index=25;
    PhysicalAddress P_A=getPhysicalAddress(mmu,L_A);
    int p_a=(P_A.frame_index<<12)|P_A.offset;
    printf("indirizzo fisico:%d con frameindex:%d e offset:%d \n\n",p_a,P_A.frame_index,P_A.offset);
*/
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
    
  
    //alloco 1 frame e lo rilascio,alloco una pagetable e la rilascio
    for (int i=0; i<20; ++i){
        MMU_process_update(mmu);
 
            FrameItem* frame=FrameEntry_create(mmu);
            printf("allocation: %d, block: %p, size: %d,next_frame_num: %d, buffer_size: %d, pt_size: %d\n", i, frame, mmu->phymem_allocator->size,mmu->phymem_allocator->first_idx,mmu->phymem_allocator->buffer_size,sizeof(frame->info)); 
            printf("releasing... idx: %d, block %p, free %d, owner: ... ",i,mmu->phy_blocks[frame->frame_num], mmu->phymem_allocator->size);
            PoolAllocatorResult release_result=PoolAllocator_releaseBlock(mmu->phymem_allocator,mmu->phy_blocks[frame->frame_num],false);
            printf("%s\n", PoolAllocator_strerror(release_result));
        
    }*/
    return 0;
}