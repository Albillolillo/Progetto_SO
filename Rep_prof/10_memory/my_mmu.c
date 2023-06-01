#include <stdio.h>
#include <stdlib.h>
#include "my_mmu.h"

//const per slab
static const int NullIdx=-1;
static const int DetachedIdx=-2;

static const char* PoolAllocator_strerrors[]=
  {"Success",
   "NotEnoughMemory",
   "UnalignedFree",
   "OutOfRange",
   "DoubleFree",
   0
  };

//******fnct MMU******

MMU* MMU_create(char* buffer_phymem,char* buffer_swapmem){
  
  //inizializzo: lista processi
  printf("Inizzializzo processi\n");
  ListProcessHead* processes=(ListProcessHead*)malloc(sizeof(ListProcessHead));
  List_init(processes);

  //inizializzo: allocatore mem. fisica
  printf("\nInizzializzo mem.fisica\n");
  PoolAllocator* phy_allocator=PoolAllocator_alloc();
  PoolAllocatorResult ret=PoolAllocator_init(phy_allocator,ITEM_SIZE,NUM_ITEMS_PHYMEM,buffer_phymem,BUFFER_SIZE_PHYMEM);
  printf("%s\n",PoolAllocator_strerror(ret));
  PoolAllocator_PrintInfo(phy_allocator);
    
  //inizializzo: allocatore mem. swap
  printf("\nInizzializzo mem.swap\n");
  PoolAllocator* swap_allocator=PoolAllocator_alloc();
  ret=PoolAllocator_init(swap_allocator,ITEM_SIZE,NUM_ITEMS_SWAPMEM,buffer_swapmem,BUFFER_SIZE_SWAPMEM);
  printf("%s\n",PoolAllocator_strerror(ret));
  PoolAllocator_PrintInfo(swap_allocator);

  MMU* mmu=(MMU*)malloc(sizeof(MMU));
  mmu->phymem_allocator=phy_allocator;
  mmu->swapmem_allocator=swap_allocator;
  mmu->MMU_processes=processes;
  mmu->curr_proc=processes->first;
  for(int i=0;i<NUM_ITEMS_PHYMEM;i++){
    mmu->phy_blocks[i]=0;
    mmu->phy_blocks_what[i]=Free_;
  }
  for(int i=0;i<NUM_ITEMS_SWAPMEM;i++){
    mmu->swap_blocks[i]=0;
    mmu->swap_blocks_what[i]=Free_;
  }

  return mmu;
};
//dato indirizzo logico ritorna inirizzo fisico
PhysicalAddress getPhysicalAddress(MMU* mmu, LogicalAddress logical_address){
  PhysicalAddress physical_address;
  physical_address.frame_index=0;
  physical_address.offset=0;

  int l_a=(logical_address.pt_index<<12)|logical_address.offset;
  printf("indirizzo logico:%d con pt_index:%d e offset:%d \n",l_a,logical_address.pt_index,logical_address.offset);
  if(l_a>=LOGMEM_MAX_SIZE || l_a<0 || logical_address.pt_index>PENTRY_NUM){
    printf("Indirizzo logico non valido\n");
    physical_address.frame_index=-1;
    physical_address.offset=-1;
    return physical_address;
  }
  int valid_bit=(mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& VALID_MASK;
  printf("valid check: %d,%d\n",mmu->curr_proc->pt->pe[logical_address.pt_index].flags,valid_bit);
  if (valid_bit==Valid){
    physical_address.frame_index= mmu->curr_proc->pt->pe[logical_address.pt_index].frame_number;
    physical_address.offset= logical_address.offset;
  }else{
    printf("pagina non valida\n");
    physical_address.frame_index=-1;
    physical_address.offset=-1;
  }
  return physical_address;
};
//scrive byte a indirizzo logico specificato
void MMU_writeByte(MMU* mmu,LogicalAddress logical_address, char c){
  PhysicalAddress physical_address;
  physical_address.frame_index=0;
  physical_address.offset=0;

  int l_a=(logical_address.pt_index<<12)|logical_address.offset;
  printf("indirizzo logico:%d con pt_index:%d e offset:%d \n",l_a,logical_address.pt_index,logical_address.offset);
  if(l_a>=LOGMEM_MAX_SIZE || l_a<0 || logical_address.pt_index>=PENTRY_NUM || logical_address.offset>=FRAME_INFO_SIZE){
    printf("Indirizzo logico non valido\n");
    return;
  }
  int valid_bit=(mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& VALID_MASK;
  printf("valid check: %d,%d\n",mmu->curr_proc->pt->pe[logical_address.pt_index].flags,valid_bit);
  if (valid_bit==Valid){
    physical_address.frame_index= mmu->curr_proc->pt->pe[logical_address.pt_index].frame_number;
    physical_address.offset= logical_address.offset;

    ((FrameItem*)mmu->phy_blocks[physical_address.frame_index])->info[physical_address.offset]=c;
    mmu->curr_proc->pt->pe[logical_address.pt_index].flags|=Write;
    printf("scritto carattere: %c ,nel frame %d all' offset %d\n",c,physical_address.frame_index,physical_address.offset);
  }else{
    printf("pagina non valida\n");
    MMU_exception(mmu,logical_address);
    MMU_writeByte(mmu,logical_address,c);
  }
  return;

};
//legge byte a indirizzo logico specificato
char* MMU_readByte(MMU* mmu,LogicalAddress logical_address){
  PhysicalAddress physical_address;
  physical_address.frame_index=0;
  physical_address.offset=0;
  char* read_byte=0;
  int l_a=(logical_address.pt_index<<12)|logical_address.offset;
  printf("indirizzo logico:%d con pt_index:%d e offset:%d \n",l_a,logical_address.pt_index,logical_address.offset);
  if(l_a>=LOGMEM_MAX_SIZE || l_a<0 || logical_address.pt_index>=PENTRY_NUM || logical_address.offset>=FRAME_INFO_SIZE){
    printf("Indirizzo logico non valido\n");
    return read_byte;
  }
  int valid_bit=(mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& VALID_MASK;
  printf("valid check: %d,%d\n",mmu->curr_proc->pt->pe[logical_address.pt_index].flags,valid_bit);
  if (valid_bit==Valid){
    physical_address.frame_index= mmu->curr_proc->pt->pe[logical_address.pt_index].frame_number;
    physical_address.offset= logical_address.offset;
    read_byte=&((FrameItem*)mmu->phy_blocks[physical_address.frame_index])->info[physical_address.offset];
    mmu->curr_proc->pt->pe[logical_address.pt_index].flags|=Read;
    printf("Byte letto");
  }else{
    printf("pagina non valida\n");
  }
  return read_byte;
};

//lancia exception se indirizzo richiesto non è valido 
void MMU_exception(MMU* mmu,LogicalAddress logical_address){
  printf("sto gestendo l'eccezione\n");
  FrameItem* requested_frame=FrameEntry_create(mmu);
  if(requested_frame){
    mmu->curr_proc->pt->pe[logical_address.pt_index].flags|=Valid;
    mmu->curr_proc->pt->pe[logical_address.pt_index].frame_number=requested_frame->frame_num;
  }else{
    printf("qualcosa va swappato\n");
  }
   
};


//aggiorna processo MMU :setta se NULL e avanza circolarmente altrimenti
void MMU_process_update(MMU* mmu){
  if(!mmu->curr_proc && mmu->MMU_processes->first){
    mmu->curr_proc=mmu->MMU_processes->first;
  }else if(mmu->curr_proc->prev){
    mmu->curr_proc=mmu->curr_proc->prev;
  }else if (!mmu->curr_proc->prev){
    mmu->curr_proc=mmu->MMU_processes->last;
  }
//aggiorno PageTablee
  for(int i=0;i<PENTRY_NUM;i++){
    PTFlags status;
    if(mmu->phy_blocks_what[mmu->curr_proc->pt->pe[i].frame_number]==Free_){
      //printf(" free frame ");
      status=Free;
    }else if(mmu->phy_blocks_what[mmu->curr_proc->pt->pe[i].frame_number]==FrameItem_){
      if(((FrameItem*)mmu->phy_blocks[mmu->curr_proc->pt->pe[i].frame_number])->pid!=mmu->curr_proc->pid){
        //printf(" frame unswappable ");
        status=Unswappable;
      }else{
        //printf(" frame valid ");
        status=Valid;
      }
    }else if(mmu->phy_blocks_what[mmu->curr_proc->pt->pe[i].frame_number]==PageTable_){
      //printf(" page unswappable ");
      status=Unswappable;
    }
    mmu->curr_proc->pt->pe[i].flags=status;
  }
};
//stampa MMU
void MMU_print(MMU* mmu){
  printf("phy_allocator:%p\nswap_allocator:%p\nLista processi:\n",mmu->phymem_allocator,mmu->swapmem_allocator);
  List_print(mmu->MMU_processes);
  printf("Processo corrente:\n");
  Process_print(mmu->curr_proc);
};



//******fnct Processo******

//alloca mem per processo
Process* Process_alloc(){
  return (Process*)malloc(sizeof(Process));
};
//libera processo *****non funziona*****
int Process_free(Process* item){
  item->next=NULL;
  item->prev=NULL;
  free(item);
  return 0;
};
//inizializza processo
void Process_init(Process* item, int pid,MMU* mmu){
  item->pid=pid;
  item->next=NULL;
  item->prev=NULL;
  item->on_disk=false;
  
  List_insert(mmu->MMU_processes,NULL,item);
  mmu->curr_proc=item;

  PageTable* pagetable=PageTable_create(mmu);
  item->pt=pagetable;

  Process_print(item);
};
//stampa un processo
void Process_print(Process* item){
  if(item){
    printf("processo:%p con pid:%d, successore:%p,predecessore:%p\n",item,item->pid,item->next,item->prev);
  }else{
    printf("non ci sono processi attivi\n");
  }
};



//******fnct PageTable******

void PageTable_init(PageTable* pt ,MMU* mmu, uint8_t frame_num){
  pt->pid=mmu->curr_proc->pid;
  pt->phymem_addr.frame_index=frame_num;
  pt->phymem_addr.offset=0;

  //printf("unswappable frame:{\n");
  for(int i=0;i<PENTRY_NUM;i++){
    pt->pe[i].frame_number=i%FRAME_NUM;
    PTFlags status=Unswappable;
    if(mmu->phy_blocks_what[pt->pe[i].frame_number]==Free_){
      pt->pe[i].flags=0;
    
    }else{
      //What_print(mmu->phy_blocks_what[pt->pe[i].frame_number]);
      if(mmu->phy_blocks_what[pt->pe[i].frame_number]==FrameItem_){
        //printf(" del processo:%d\n",((FrameItem*)mmu->phy_blocks[pt->pe[i].frame_number])->pid);
        if(((FrameItem*)mmu->phy_blocks[pt->pe[i].frame_number])->pid==pt->pid){
          //printf("Mio frame\n");
          pt->pe[i].flags=0;
        }else{
          //printf("busy_frame:%d\n",pt->pe[i].frame_number);
          //printf("%p\n",mmu->phy_blocks[pt->pe[i].frame_number]);
          pt->pe[i].flags=status;
        }
      }else if(mmu->phy_blocks_what[pt->pe[i].frame_number]==PageTable_){
        if(mmu->phy_blocks[pt->pe[i].frame_number]){
          //printf(" del processo:%d\n",((PageTable*)mmu->phy_blocks[pt->pe[i].frame_number])->pid);
        }else{
          //printf(" del processo:%d\n",((PageTable*)mmu->phy_blocks[pt->pe[i].frame_number-1])->pid);
        }
        //printf("busy_frame:%d\n",pt->pe[i].frame_number);
        //printf("%p\n",mmu->phy_blocks[pt->pe[i].frame_number]);
        pt->pe[i].flags=status;
      }
      
    }
    }
    printf("}\n");
}


void PageTable_print(PageTable* pt ){
  printf("\npid:%d, frame_num:%d, phymem_addr:%d\n",pt->pid,pt->phymem_addr.frame_index,pt->phymem_addr);
  for(int i=0;i<PENTRY_NUM;i++){
    printf("PageEntry_num:%d -> %d-%d ",i,pt->pe[i].flags,pt->pe[i].frame_number);
    //if(i>30)break;//toglirere per vedere tutto
  }
};
//sugar fnct
PageTable* PageTable_create(MMU* mmu){
  int frame_num=mmu->phymem_allocator->first_idx;
  PageTable* pagetable=(PageTable*)PoolAllocator_getBlock(mmu,true);
  //mmu->phy_blocks[frame_num]=pagetable;^^ ora viene fatto dentro ^^
  mmu->phy_blocks_what[frame_num]=PageTable_;
  mmu->phy_blocks_what[frame_num+1]=PageTable_;
  printf("\nPagetable:%p\n",pagetable);
  if(mmu->phy_blocks[frame_num]){
    PageTable_init(pagetable,mmu,frame_num); 
    //PageTable_print((PageTable*)mmu->phy_blocks[frame_num]);
  }
  return pagetable;
}

void PageTable_release(MMU*mmu,int block_index){
  PoolAllocatorResult release_result=PoolAllocator_releaseBlock(mmu->phymem_allocator,mmu->phy_blocks[block_index],true);
  mmu->phy_blocks_what[block_index]=Free_;
  mmu->phy_blocks_what[block_index+1]=Free_;
  printf("%s\n", PoolAllocator_strerror(release_result));
}



//******fnct FrameItem******

void FrameEntry_init(FrameItem* item, int pid, uint32_t frame_num){
  for(int i=0;i<FRAME_INFO_SIZE;i++){
    item->info[i]=0;
  }  
  
  item->pid=pid;
  item->frame_num=frame_num;
}

void FrameEntry_print(FrameItem* item){
  if(item){
    printf("pid: %d, frame: %d, info: ", item->pid, item->frame_num);
    for(int i=0;i<sizeof(item->info);i++){
      printf("%c",item->info[i]);
    }
  }
}
//sugar fnct
FrameItem* FrameEntry_create(MMU* mmu){
  int frame_num=mmu->phymem_allocator->first_idx;
  FrameItem* frame=(FrameItem*)PoolAllocator_getBlock(mmu,false);
  //mmu->phy_blocks[frame_num]=frame;^^ ora viene fatto dentro ^^
  mmu->phy_blocks_what[frame_num]=FrameItem_;
  printf("\nframe:%p\n",frame);
  if(frame){
    FrameEntry_init(frame,mmu->curr_proc->pid,frame_num);

    for(int i=0;i<PENTRY_NUM;i++){
      if(mmu->curr_proc->pt->pe[i].frame_number==frame_num && (mmu->curr_proc->pt->pe[i].flags&VALID_MASK)!=Valid){
        mmu->curr_proc->pt->pe[i].flags= (mmu->curr_proc->pt->pe[i].flags|VALID_MASK);
        //printf("le PageEntry del processo corrente che puntano al frame %d sono state settate a valid: %d\n",frame_num,mmu->curr_proc->pt->pe[i].flags);
      }
    }
    FrameEntry_print((FrameItem*)mmu->phy_blocks[frame_num]);
  }
  return frame;
}

void Frame_release(MMU*mmu,int block_index){
  PoolAllocatorResult release_result=PoolAllocator_releaseBlock(mmu->phymem_allocator,mmu->phy_blocks[block_index],false);
  mmu->phy_blocks_what[block_index]=Free_;
  printf("%s\n", PoolAllocator_strerror(release_result));
}



//******fnct List******

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

int List_free(ListProcessHead* head){
  Process* aux=head->first;
  while(aux){
    Process_free(aux);
  }
  return 0;
}



//******fnct SLAB******

const char* PoolAllocator_strerror(PoolAllocatorResult result) {
  return PoolAllocator_strerrors[-result];
}

PoolAllocator* PoolAllocator_alloc(){
  return (PoolAllocator*)malloc(sizeof(PoolAllocator));
}

PoolAllocatorResult PoolAllocator_init(PoolAllocator* a,int item_size,int num_items,char* memory_block,int memory_size) {

  // we first check if we have enough memory
  // for the bookkeeping
  int requested_size= num_items*(item_size+sizeof(int));
  if (memory_size<requested_size)
    return NotEnoughMemory;

  a->item_size=item_size;
  a->size=num_items;
  a->buffer_size=item_size*num_items;
  a->size_max = num_items;
  
  a->buffer=memory_block; // the upper part of the buffer is used as memory
  a->free_list= (int*)(memory_block+item_size*num_items); // the lower part is for bookkeeping

  // now we populate the free list by constructing a linked list
  for (int i=0; i<a->size-1; ++i){
    a->free_list[i]=i+1;
  }
  // set the last element to "NULL" 
  a->free_list[a->size-1] = NullIdx;
  a->first_idx=0;
  return Success;
}

void* PoolAllocator_getBlock(MMU*mmu,bool which) {
  if (mmu->phymem_allocator->first_idx==-1)
    return 0;

  // we need to remove the first bucket from the list
  int detached_idx = mmu->phymem_allocator->first_idx;

  //se which==0 voglio un frame altrimenti voglio una PageTable
if(!which){
  // advance the head
  mmu->phymem_allocator->first_idx=mmu->phymem_allocator->free_list[mmu->phymem_allocator->first_idx];
  --mmu->phymem_allocator->size;
  
  mmu->phymem_allocator->free_list[detached_idx]=DetachedIdx;
  
  //now we retrieve the pointer in the item buffer
  char* block_address=mmu->phymem_allocator->buffer+(detached_idx*mmu->phymem_allocator->item_size);
  mmu->phy_blocks[detached_idx]=(FrameItem*)block_address;//!
  return block_address;
}else if (which && mmu->phymem_allocator->size>3){
  // advance the head
  
  mmu->phymem_allocator->first_idx=mmu->phymem_allocator->free_list[mmu->phymem_allocator->first_idx];
  mmu->phymem_allocator->first_idx=mmu->phymem_allocator->free_list[mmu->phymem_allocator->first_idx];
  --mmu->phymem_allocator->size;
  --mmu->phymem_allocator->size;
  mmu->phymem_allocator->free_list[detached_idx]=DetachedIdx;
  mmu->phymem_allocator->free_list[detached_idx+1]=DetachedIdx;
  
  //now we retrieve the pointer in the item buffer
  char* block_address=mmu->phymem_allocator->buffer+(detached_idx*mmu->phymem_allocator->item_size);

  mmu->phy_blocks[detached_idx]=(PageTable*)block_address;//!
  return block_address;
}else {
  return 0;
}
}

PoolAllocatorResult PoolAllocator_releaseBlock(PoolAllocator* a, void* block_,bool which){
  if(which){
    //we need to find the index from the address
    char* block=(char*) block_;
    int offset=block - a->buffer;
    //sanity check, we need to be aligned to the block boundaries
    if (offset%a->item_size)
      return UnalignedFree;

    int idx=offset/a->item_size;

    //sanity check, are we inside the buffer?
    if (idx+1<0 || idx+1>=a->size_max)
          return OutOfRange;

    //is the block detached?
    if (a->free_list[idx+1]!=DetachedIdx)
      return DoubleFree;

    a->free_list[idx+1]=a->first_idx;
    a->first_idx=idx+1;
    ++a->size;
    
    //sanity check, are we inside the buffer?
    if (idx<0 || idx>=a->size_max)
      return OutOfRange;

    //is the block detached?
    if (a->free_list[idx]!=DetachedIdx)
      return DoubleFree;    

    // all fine, we insert in the head
    a->free_list[idx]=a->first_idx;
    a->first_idx=idx;
    ++a->size;

    return Success;
  
  }else{
    //we need to find the index from the address
    char* block=(char*) block_;
    int offset=block - a->buffer;
    //sanity check, we need to be aligned to the block boundaries
    if (offset%a->item_size)
      return UnalignedFree;

    int idx=offset/a->item_size;

    //sanity check, are we inside the buffer?
    if (idx<0 || idx>=a->size_max)
      return OutOfRange;

    //is the block detached?
    if (a->free_list[idx]!=DetachedIdx)
      return DoubleFree;

      // all fine, we insert in the head
    
    a->free_list[idx]=a->first_idx;
    a->first_idx=idx;
    ++a->size;
    return Success;
  }
}

void PoolAllocator_PrintInfo(PoolAllocator* a){
  printf("item_size:%d\n num_free_block:%d\n buf_addr:%p\n buffer_size(Bytes):%d\n free_list addr:%p\n max_size:%d\n first_bucket_idx:%d\n ",
          a->item_size,a->size,a->buffer,a->buffer_size,a->free_list,a->size_max,a->first_idx);
}



void What_print(what_Flag what){
  switch (what){
  case 0x1:
    printf("Frame libero");
    break;
  case 0x2:
    printf("Frame entry");
    break;
  case 0x3:
    printf("PageTable");
    break;
  }
}