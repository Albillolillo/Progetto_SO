#include <stdio.h>
#include "my_mmu.h"



MMU* MMU_init(PoolAllocator phymem_allocator,PoolAllocator swapmem_allocator){};
//dato indirizzo logico ritorna inirizzo fisico
PhysicalAddress getPhysicalAddress(MMU* mmu, LogicalAddress logical_address){};
//scrive byte a indirizzo logico specificato
void MMU_writeByte(MMU* mmu,LogicalAddress pos, char c){};
//legge byte a indirizzo logico specificato
char* MMU_readByte(MMU* mmu,LogicalAddress pos){};
//lancia exception se indirizzo richiesto non è valido 
void MMU_exception(MMU* mmu, int pos){};





//fnct Processo

//alloca mem per processo
Process* Process_alloc(){};
//libera processo
int Process_free(Process* item){};
//inizializza processo
void Process_init(Process* item, int pid){};
//stampa un processo
void Process_print(Process* item){};



//fnct PageTable

//inizzializza PageTable
PageTable PageTable_init(){};
//stampa PageTable
void PageTable_print(){};

FrameItem* FrameItem_alloc() {
  return (FrameItem*) malloc (sizeof(FrameItem));
}

int FrameItem_free(FrameItem* item) {
  free(item);
  return 0;
}

void FrameItem_init(FrameItem* item, int pid, uint32_t frame_num){
  for(int i=0;i<FRAME_INFO_SIZE;i++){
    item->info[i]=0;
  }  
  item->list.prev=item->list.next=0;
  item->pid=pid;
  item->frame_num=frame_num;
}

void FrameItem_print(FrameItem* item){
  printf("(pid: %d, frame: %d)", item->pid, item->frame_num);
}

void FrameList_print(ListHead* list) {
  ListItem* aux=(ListItem*) list->first;
  while(aux){
    FrameItem_print((FrameItem*) aux);
    aux=aux->next;
    if(aux)
      printf(", ");
  }
  
}