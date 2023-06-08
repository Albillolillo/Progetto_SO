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
    
    
    //TEST 1 
    //Creo 10 processi, stampo MMU e la lista dei processi
    for (int i=1;i<11;i++){
        Process *item=Process_alloc();
        Process_init(item,i,mmu);
        
    }

    MMU_print(mmu);
    MMU_process_update(mmu);
    List_print(mmu->MMU_processes);


/*
    LogicalAddress L_A;
    for (int i=0; i<201; ++i){
        
        L_A.offset=(i*2)%PENTRY_NUM;
        L_A.pt_index=i;
        char c=i%FRAME_NUM;
        MMU_writeByte(mmu,L_A,c);
        
        char* read_byte=MMU_readByte(mmu,L_A);
    
        if(read_byte){
            printf(": %c\n",*read_byte);
        }
        if(i%20==0){
            MMU_process_update(mmu);
        }
    }
     
    for (int i=0; i<60; ++i){
        
        L_A.offset=i;
        L_A.pt_index=(i)%PENTRY_NUM;
        char c=i%FRAME_NUM;
        MMU_writeByte(mmu,L_A,c);
        if(i%2==0){
            char* read_byte=MMU_readByte(mmu,L_A);
        }
    }

    PageTable_print(mmu->curr_proc->pt);

    for (int i=0; i<300; ++i){
        
        L_A.offset=i;
        L_A.pt_index=(i)%PENTRY_NUM;
        char c=i%FRAME_NUM;
        MMU_writeByte(mmu,L_A,c);
        if(i%2==0){
            char* read_byte=MMU_readByte(mmu,L_A);
        }
        
    }
     PageTable_print(mmu->curr_proc->pt);

     for (int i=0; i<300; ++i){
        
        L_A.offset=i;
        L_A.pt_index=(i)%PENTRY_NUM;
        char c=i%FRAME_NUM;
        MMU_writeByte(mmu,L_A,c);
        if(i%2==0){
            char* read_byte=MMU_readByte(mmu,L_A);
        }
    }

    PageTable_print(mmu->curr_proc->pt);

    Process_release(mmu->MMU_processes->last->prev->prev,mmu);
    List_print(mmu->MMU_processes);
    Process *item=Process_alloc();
    Process_init(item,6,mmu);
    Process_init(item,3,mmu);
    List_print(mmu->MMU_processes);
*/
     /*
    int count=0;
     for(int i=0;i<4096;i++){
        
        if(mmu->swap_blocks[i]){
           printf("%d",((FrameItem*)mmu->swap_blocks[i])->pid); 
           count++;
        }
        
    }
    printf("\n%d",_count); 
    
    MMU_process_update(mmu);

    for (int i=0; i<4120; ++i){
        
        L_A.offset=i%FRAME_INFO_SIZE;
        L_A.pt_index=i%FRAME_NUM;
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

    */

    return 0;
}