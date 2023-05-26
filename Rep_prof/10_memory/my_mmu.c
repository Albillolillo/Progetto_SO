#include <stdio.h>
#include <stdlib.h>
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
Process* Process_alloc(){
  return (Process*)malloc(sizeof(Process));
};
//libera processo
int Process_free(Process* item){};
//inizializza processo
void Process_init(Process* item, int pid){
  item->pid=pid;
  item->next=NULL;
  item->prev=NULL;
  item->on_disk=false;
};

//stampa un processo
void Process_print(Process* item){
    printf("processo:%p con pid:%d, successore:%p,predecessore:%p\n",item,item->pid,item->next,item->prev);
}



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
  //item->prev=item->next=0;
  item->pid=pid;
  item->frame_num=frame_num;
}

void FrameItem_print(FrameItem* item){
  printf("(pid: %d, frame: %d)", item->pid, item->frame_num);
}

void FrameList_print(ListProcessHead* list) {
  Process* aux=(Process*) list->first;
  while(aux){
    FrameItem_print((FrameItem*) aux);
    aux=aux->next;
    if(aux)
      printf(", ");
  }
  
}






void List_init(ListProcessHead* head) {
  head->first=0;
  head->last=0;
  head->size=0;
}

Process* List_find(ListProcessHead* head, Process* item) {
  // linear scanning of list
  Process* aux=head->first;
  while(aux){
    if (aux==item)
      return item;
    aux=aux->next;
  }
  return 0;
}

Process* List_insert(ListProcessHead* head, Process* prev, Process* item) {
  if (item->next || item->prev)
    return 0;

  Process* next= prev ? prev->next : head->first;
  if (prev) {
    item->prev=prev;
    prev->next=item;
  }
  if (next) {
    item->next=next;
    next->prev=item;
  }
  if (!prev)
    head->first=item;
  if(!next)
    head->last=item;
  ++head->size;
  return item;
}

Process* List_detach(ListProcessHead* head, Process* item) {

  Process* prev=item->prev;
  Process* next=item->next;
  if (prev){
    prev->next=next;
  }
  if(next){
    next->prev=prev;
  }
  if (item==head->first)
    head->first=next;
  if (item==head->last)
    head->last=prev;
  head->size--;
  item->next=item->prev=0;
  return item;
}
void List_print(ListProcessHead* head){
    Process* aux=head->first;
  while(aux){
      Process_print(aux);
      aux=aux->next;
  }
}
