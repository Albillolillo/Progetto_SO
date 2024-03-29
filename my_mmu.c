#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

//Creo MMU
MMU* MMU_create(char* buffer_phymem,char* buffer_swapmem){
  
  //alloco lista processi
  printf("Inizzializzo processi\n");
  ListProcessHead* processes=(ListProcessHead*)malloc(sizeof(ListProcessHead));
  List_init(processes);

  //alloco allocatore mem. fisica
  printf("\nInizzializzo mem.fisica\n");
  PoolAllocator* phy_allocator=PoolAllocator_alloc();
  PoolAllocatorResult ret=PoolAllocator_init(phy_allocator,ITEM_SIZE,NUM_ITEMS_PHYMEM,buffer_phymem,BUFFER_SIZE_PHYMEM);
  printf("%s\n",PoolAllocator_strerror(ret));
  PoolAllocator_PrintInfo(phy_allocator);
    
  //alloco allocatore mem. swap
  printf("\nInizzializzo mem.swap\n");
  PoolAllocator* swap_allocator=PoolAllocator_alloc();
  ret=PoolAllocator_init(swap_allocator,ITEM_SIZE,NUM_ITEMS_SWAPMEM,buffer_swapmem,BUFFER_SIZE_SWAPMEM);
  printf("%s\n",PoolAllocator_strerror(ret));
  PoolAllocator_PrintInfo(swap_allocator);

  //alloco MMU e inizializzo tutti i campi
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
    mmu->swap_blocks_what[i].what=Free_;
    mmu->swap_blocks_what[i].from_where=0;
  }

  return mmu;
};

//Dato indirizzo logico ritorna inirizzo fisico (funzione utilizzata solo in fase di progettazione)
PhysicalAddress getPhysicalAddress(MMU* mmu, LogicalAddress logical_address){

  //inizializzo indirizzo di ritorno
  PhysicalAddress physical_address;
  physical_address.frame_index=0;
  physical_address.offset=0;

  //calcolo indirizzo logico e verifico che sia nei limiti
  int l_a=(logical_address.pt_index<<12)|logical_address.offset;
  printf("\nindirizzo logico:%d con pt_index:%d e offset:%d \n",l_a,logical_address.pt_index,logical_address.offset);
  if(l_a>=LOGMEM_MAX_SIZE || l_a<0 || logical_address.pt_index>PENTRY_NUM){
    printf("Indirizzo logico non valido\n");
    physical_address.frame_index=-1;
    physical_address.offset=-1;
    return physical_address;
  }

  //controllo se la page entry richiesta è valida: si,formo indirizzo fisico valido; no,formo indirizzo fisico non valido. Ritorno indirizzo formato 
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
  //inizializzo indirizzo fisico
  PhysicalAddress physical_address;
  physical_address.frame_index=0;
  physical_address.offset=0;

  //se indirizzo logico non è valido return
  int l_a=(logical_address.pt_index<<12)|logical_address.offset;
  printf("\nindirizzo logico:%d con pt_index:%d e offset:%d \n",l_a,logical_address.pt_index,logical_address.offset);
  if(l_a>=LOGMEM_MAX_SIZE || l_a<0 || logical_address.pt_index>=PENTRY_NUM || logical_address.offset>=FRAME_INFO_SIZE){
    printf("Indirizzo logico non valido\n");
    return;
  }

  //indirizzo è valido e non unswappable (è un Frame), formo indirizzo fisico, scrivo e aggiorno i flags della PageTable
  printf("valid check: %d\n",mmu->curr_proc->pt->pe[logical_address.pt_index].flags);
  if ((((mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& VALID_MASK)==Valid)&& !(((mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& UNSWAPPABLE_MASK)==Unswappable)){
    physical_address.frame_index= mmu->curr_proc->pt->pe[logical_address.pt_index].frame_number;
    physical_address.offset= logical_address.offset;
    ((FrameItem*)mmu->phy_blocks[physical_address.frame_index])->info[physical_address.offset]=c;
    mmu->curr_proc->pt->pe[logical_address.pt_index].flags|=Write;
    mmu->curr_proc->last_changed_index=logical_address.pt_index;
    printf("scritto carattere: %c ,nel frame %d all' offset %d , ora i flags sono:%d\n",c,physical_address.frame_index,physical_address.offset,mmu->curr_proc->pt->pe[logical_address.pt_index].flags);
  
  //indirizzo è valido e unswappable (è una PageTable)
  }else if (((mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& VALID_MASK)==Valid){
    printf("non puoi scrivere sulla tua PageTable\n");

  //indirizzo è swapped, per scrivere su questo frame devo riportarlo in memoria fisica 
  }else if(((mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& SWAPPED_MASK)==Swapped){
    //trovo una vittima per fare spazio al frame 
    printf("L'indirizzo richiesto sulla swapmem, cerco frame da sostituire\n");
    aux_struct index=FindVictim(mmu->curr_proc);
    if (index.index==255 && index.pt_index==4096){
      printf("Il processo corrente non ha frame swappable\n");
    }else{
      //vittima trovata provo lo swapout del frame
      printf("\nvictim index: %d\n",index.index);    
      int done=SwapOut_Frame(mmu,index);
      if(done=1){
        //ripristino il frame in phymem ,scrivo e libero il frame nella swapmem
        int swapin_index=FindAddress(mmu,logical_address.pt_index);
        printf("\nIl contenuto della pEntry %d si trovava nel frame %d della swapmem\n",logical_address.pt_index,swapin_index);  
        if(mmu->curr_proc->pid==((FrameItem*)mmu->swap_blocks[swapin_index])->pid){
          FrameItem* backin_frame=FrameEntry_create(mmu);
          mmu->curr_proc->pt->pe[logical_address.pt_index].flags=Valid;
          mmu->curr_proc->pt->pe[logical_address.pt_index].frame_number=backin_frame->frame_num;
          memcpy(backin_frame->info, ((FrameItem*)mmu->swap_blocks[swapin_index])->info, FRAME_INFO_SIZE);
          printf("\ni flags della entry %d ora sono %d \n",logical_address.pt_index,mmu->curr_proc->pt->pe[logical_address.pt_index].flags);
          FrameEntry_print(backin_frame);
          Frame_release(mmu,swapin_index,true);
          MMU_writeByte(mmu,logical_address,c);
        }
      }
    }

  //indirizzo non è valido, page fault, chiamo MMU_exception 
  }else{
    printf("pagina non valida\n");
    FrameItem* new_frame=MMU_exception(mmu,logical_address);

    //MMU_exception è andata a buon fine, scrivo su frame restituito
    if(new_frame){
      MMU_writeByte(mmu,logical_address,c);

    //MMU_exception è fallita, scrittura negata
    }else{
      printf("Scrittura NEGATA\n");
    }
  };
}

//legge byte a indirizzo logico specificato
char* MMU_readByte(MMU* mmu,LogicalAddress logical_address){
  //inizializzo indirizzo fisico e puntatore da ritornare
  PhysicalAddress physical_address;
  physical_address.frame_index=0;
  physical_address.offset=0;
  char* read_byte=0;

  //se indirizzo logico non è valido return
  int l_a=(logical_address.pt_index<<12)|logical_address.offset;
  printf("\nindirizzo logico:%d con pt_index:%d e offset:%d \n",l_a,logical_address.pt_index,logical_address.offset);
  if(l_a>=LOGMEM_MAX_SIZE || l_a<0 || logical_address.pt_index>=PENTRY_NUM || logical_address.offset>=FRAME_INFO_SIZE){
    printf("Indirizzo logico non valido\n");
    return read_byte;
  }

  //indirizzo è valido e non unswappable (è un Frame), formo indirizzo fisico, leggo e aggiorno i flags della PageTable
  printf("valid check: %d\n",mmu->curr_proc->pt->pe[logical_address.pt_index].flags);
  if ((((mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& VALID_MASK)==Valid)&& !(((mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& UNSWAPPABLE_MASK)==Unswappable)){
    physical_address.frame_index= mmu->curr_proc->pt->pe[logical_address.pt_index].frame_number;
    physical_address.offset= logical_address.offset;
    read_byte=&((FrameItem*)mmu->phy_blocks[physical_address.frame_index])->info[physical_address.offset];
    mmu->curr_proc->pt->pe[logical_address.pt_index].flags|=Read;
    mmu->curr_proc->last_changed_index=logical_address.pt_index;
    printf("Byte letto ora i flags sono:%d",mmu->curr_proc->pt->pe[logical_address.pt_index].flags);

  //indirizzo è valido e unswappable (è una PageTable)
  }else if(((mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& VALID_MASK)==Valid){
    printf("non posso leggere una PageTable");

  //indirizzo è swapped, per leggere da questo frame devo riportarlo in memoria fisica 
  }else if(((mmu->curr_proc->pt->pe[logical_address.pt_index].flags)& SWAPPED_MASK)==Swapped){
    //trovo una vittima per fare spazio al frame
    printf("L'indirizzo richiesto sulla swapmem, cerco frame da sostituire\n");
    aux_struct index=FindVictim(mmu->curr_proc);
    if (index.index==255 && index.pt_index==4096){
      printf("Il processo corrente non ha frame swappable\n");
    }else{
      //vittima trovata provo lo swapout del frame
      printf("\nvictim index: %d\n",index.index);    
      int done=SwapOut_Frame(mmu,index);
      if(done=1){
        //ripristino il frame in phymem ,scrivo e libero il frame nella swapmem
        int swapin_index=FindAddress(mmu,logical_address.pt_index);
        printf("\nIl contenuto della pEntry %d si trovava nel frame %d della swapmem\n",logical_address.pt_index,swapin_index);  
        if(mmu->curr_proc->pid==((FrameItem*)mmu->swap_blocks[swapin_index])->pid){
          FrameItem* backin_frame=FrameEntry_create(mmu);
          mmu->curr_proc->pt->pe[logical_address.pt_index].flags=Valid;
          mmu->curr_proc->pt->pe[logical_address.pt_index].frame_number=backin_frame->frame_num;
          memcpy(backin_frame->info, ((FrameItem*)mmu->swap_blocks[swapin_index])->info, FRAME_INFO_SIZE);
          printf("\ni flags della entry %d ora sono %d \n",logical_address.pt_index,mmu->curr_proc->pt->pe[logical_address.pt_index].flags);
          FrameEntry_print(backin_frame);
          Frame_release(mmu,swapin_index,true);
          return MMU_readByte(mmu,logical_address);
        }
      }
    }
  
  //indirizzo è non valido
  }else {
    printf("pagina non valida\n");
  }

  return read_byte;
};

//lancia exception se indirizzo richiesto in scrittura non è valido, page fault
FrameItem* MMU_exception(MMU* mmu,LogicalAddress logical_address){
  printf("sto gestendo l'eccezione\n");
  FrameItem* requested_frame=FrameEntry_create(mmu);

  //c'è ancora spazio nella phymem, aggiorno page entry
  if(requested_frame){
    mmu->curr_proc->pt->pe[logical_address.pt_index].flags=Valid;
    mmu->curr_proc->pt->pe[logical_address.pt_index].frame_number=requested_frame->frame_num;
    
  //phymem è piena, è necessario fare swapout di un frame 
  }else{
    //cerco vittima 
    printf("Phymem piena, swap necessario\n");
    aux_struct index=FindVictim(mmu->curr_proc);
    //non trovata
    if (index.index==255 && index.pt_index==4096){
      printf("Il processo corrente non ha frame swappable\n");
    //trovata
    }else{
      printf("\nvictim index: %d\n",index.index);    
      int done=SwapOut_Frame(mmu,index);
      if(done=1){
        //swapout riuscito richiedo creazione frame
        requested_frame=FrameEntry_create(mmu);
        mmu->curr_proc->pt->pe[logical_address.pt_index].flags=Valid;
        mmu->curr_proc->pt->pe[logical_address.pt_index].frame_number=requested_frame->frame_num;
      }
    }
  }
  return requested_frame;
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

//dealloca processo 
int Process_free(Process* item){
  item->next=NULL;
  item->prev=NULL;
  free(item);
  return 0;
};

//inizializza processo
void Process_init(Process* item, int pid,MMU* mmu){
  //cerco pid e processo per vedere se esiste già
  Process* find_proc=List_find(mmu->MMU_processes,item);
  Process* find_pid=List_find_pid(mmu->MMU_processes,pid);
  printf("trovato processo:%p, in pid inserito appartiene già al processo:%p\n",find_proc,find_pid);

  //processo non esiste
  if(find_proc==0 && find_pid==0){
    item->pid=pid;
    item->next=NULL;
    item->prev=NULL;
    item->last_changed_index=0;
    
    List_insert(mmu->MMU_processes,NULL,item);
    mmu->curr_proc=item;

    PageTable* pagetable=PageTable_create(mmu);
    item->pt=pagetable;
    Process_print(item);

  //processo esiste
  }else{
    printf("processo con pid %d esiste già\n",pid);
  }
};

//libera processo rilasciando Frames e PageTable
void Process_release(Process* item,MMU* mmu){
  //cerco il processo nella lista
  Process* victim=List_find(mmu->MMU_processes,item);
  printf("\nsto liberando processo:%d\n",victim->pid);

  //stacco processo dalla lista
  List_detach(mmu->MMU_processes,item);

  //rilascio i frame che possiede in phymem
  for(int i=0;i<FRAME_NUM-1;i++){
    if (mmu->phy_blocks_what[i]==FrameItem_){
      if(((FrameItem*)mmu->phy_blocks[i])->pid==victim->pid){
        printf("  %d  ",i);
        printf("rilasciato Frame da phy\n");
        Frame_release(mmu,i,false);
      }
    }else if(mmu->phy_blocks_what[i]==PageTable_){
      if(((PageTable*)mmu->phy_blocks[i])->pid==victim->pid){
        printf("  %d  ",i);
        printf("rilasciata PageTable da phy\n");
        PageTable_release(mmu,i);
        
      }
      i++;
    }
  }

  //rilascio i frame che possiede in swapmem
  for(int i=0;i<PENTRY_NUM;i++){
    if (mmu->swap_blocks_what[i].what==FrameItem_){
      if(((FrameItem*)mmu->swap_blocks[i])->pid==victim->pid){
        printf("  %d  ",i);
        printf("rilasciato Frame da swap\n");
        Frame_release(mmu,i,true);
        mmu->swap_blocks_what[i].what==Free_;
      }
    }else if(mmu->swap_blocks_what[i].what==PageTable_){
      if(((PageTable*)mmu->swap_blocks[i])->pid==victim->pid){
        printf("  %d  ",i);
        printf("rilasciata PageTable da swap\n");
        PageTable_release(mmu,i);
      }
      i++;
    }    
  }

  //se sto liberando il processo corrente aggiorno il processo corrente
  if (victim==mmu->curr_proc){
    MMU_process_update(mmu);
  }

  Process_free(item);
}

//stampa un processo
void Process_print(Process* item){
  if(item){
    printf("processo:%p con pid:%d, successore:%p,predecessore:%p\n",item,item->pid,item->next,item->prev);
  }else{
    printf("non ci sono processi attivi\n");
  }
};



//******fnct PageTable******


//inizializza PageTable
void PageTable_init(PageTable* pt ,MMU* mmu, uint8_t frame_num){
  //inizializzo campi
  pt->pid=mmu->curr_proc->pid;
  pt->phymem_addr.frame_index=frame_num;
  pt->phymem_addr.offset=0;

  //inizializza flags (varie print interne disattivate)
  printf("frame_flags:{\n");
  for(int i=0;i<PENTRY_NUM;i++){
    pt->pe[i].frame_number=i%(FRAME_NUM-1);
    PTFlags status=Unswappable;
    if(mmu->phy_blocks_what[pt->pe[i].frame_number]==Free_){
      pt->pe[i].flags=0;
      continue;
    }else{
      //What_print(mmu->phy_blocks_what[pt->pe[i].frame_number]);
      if(mmu->phy_blocks_what[pt->pe[i].frame_number]==FrameItem_){
        //printf(" del processo:%d\n",((FrameItem*)mmu->phy_blocks[pt->pe[i].frame_number])->pid);
        if(((FrameItem*)mmu->phy_blocks[pt->pe[i].frame_number])->pid==pt->pid){
          //printf("Mio frame\n");
          pt->pe[i].flags=Valid;
          continue;
        }else{
          //printf("busy_frame:%d\n",pt->pe[i].frame_number);
          //printf("%p\n",mmu->phy_blocks[pt->pe[i].frame_number]);
          pt->pe[i].flags=status;
          continue;
        }
      }else if(mmu->phy_blocks_what[pt->pe[i].frame_number]==PageTable_){
        if(mmu->phy_blocks[pt->pe[i].frame_number]){
          pt->pe[i+1].frame_number=(i+1)% (FRAME_NUM-1);
          if(((PageTable*)mmu->phy_blocks[pt->pe[i].frame_number])->pid==pt->pid){
            pt->pe[i].flags=status;
            pt->pe[i].flags|=VALID_MASK;
            pt->pe[i+1].flags=status;
            pt->pe[i+1].flags|=VALID_MASK;
          //printf(" del processo:%d\n",((PageTable*)mmu->phy_blocks[pt->pe[i].frame_number])->pid);
          }else{
            pt->pe[i].flags=status;
            pt->pe[i+1].flags=status;
          }
        i++;
        //printf("busy_frame:%d\n",pt->pe[i].frame_number);
        //printf("%p\n",mmu->phy_blocks[pt->pe[i].frame_number]);
        }
      }
    }
  }
  printf("}\n");
}

//stampa PageTable
void PageTable_print(PageTable* pt ){
  printf("\npid:%d, frame_location:%d, phymem_addr:%d\n",pt->pid,pt->phymem_addr.frame_index,pt->phymem_addr);
  for(int i=0;i<PENTRY_NUM;i++){
    printf("PageEntry_num:%d -> %d-%d ",i,pt->pe[i].flags,pt->pe[i].frame_number);
  }
};

//richiede blocco per la PageTable (al phymem_allocator), la inizializza e aggiorna phy_blocks_what[]
PageTable* PageTable_create(MMU* mmu){
  int frame_num=mmu->phymem_allocator->first_idx;
  PageTable* pagetable=(PageTable*)PoolAllocator_getBlock(mmu,true,false);
  mmu->phy_blocks_what[frame_num]=PageTable_;
  mmu->phy_blocks_what[frame_num+1]=PageTable_;
  printf("\nPagetable:%p\n",pagetable);
  if(mmu->phy_blocks[frame_num]){
    PageTable_init(pagetable,mmu,frame_num); 
    //PageTable_print((PageTable*)mmu->phy_blocks[frame_num]);
  }
  return pagetable;
}

//rilascia PageTable dalla phymem
void PageTable_release(MMU*mmu,int block_index){
  PoolAllocatorResult release_result=PoolAllocator_releaseBlock(mmu->phymem_allocator,mmu->phy_blocks[block_index],true);
  mmu->phy_blocks_what[block_index]=Free_;
  mmu->phy_blocks_what[block_index+1]=Free_;
  printf("%s\n", PoolAllocator_strerror(release_result));
}



//******fnct FrameItem******

//inizializza Frame
void FrameEntry_init(FrameItem* item, int pid, uint32_t frame_num){
  for(int i=0;i<FRAME_INFO_SIZE;i++){
    item->info[i]=0;
  }
  item->pid=pid;
  item->frame_num=frame_num;
}

//stampa Frame
void FrameEntry_print(FrameItem* item){
  if(item){
    printf("pid: %d, frame: %d, info: ", item->pid, item->frame_num);
    for(int i=0;i<sizeof(item->info);i++){
      printf("%c",item->info[i]);
    }
    printf("\n");
  }
}

//richiede blocco per la Frame (al phymem_allocator), lo inizializza e aggiorna phy_blocks_what[]
FrameItem* FrameEntry_create(MMU* mmu){
  int frame_num=mmu->phymem_allocator->first_idx;
  FrameItem* frame=(FrameItem*)PoolAllocator_getBlock(mmu,false,false);
  mmu->phy_blocks_what[frame_num]=FrameItem_;
  printf("frame:%p\n",frame);
  if(frame){
    FrameEntry_init(frame,mmu->curr_proc->pid,frame_num);
    FrameEntry_print((FrameItem*)mmu->phy_blocks[frame_num]);
  }
  return frame;
}

//rilascia Frame dalla phymem se where==false o dalla swapmem se where==true
void Frame_release(MMU*mmu,int block_index,bool where){
  if(!where){
    PoolAllocatorResult release_result=PoolAllocator_releaseBlock(mmu->phymem_allocator,mmu->phy_blocks[block_index],false);
    mmu->phy_blocks_what[block_index]=Free_;

    int frame_num=((FrameItem*)mmu->phy_blocks[block_index])->frame_num;
    FrameEntry_init(mmu->phy_blocks[block_index],0,frame_num);
    printf("%s\n", PoolAllocator_strerror(release_result));
  }else{
    PoolAllocatorResult release_result=PoolAllocator_releaseBlock(mmu->swapmem_allocator,mmu->swap_blocks[block_index],false);
    mmu->swap_blocks_what[block_index].what=Free_;
    mmu->swap_blocks_what[block_index].from_where=0;

    int frame_num=((FrameItem*)mmu->swap_blocks[block_index])->frame_num;
    FrameEntry_init(mmu->swap_blocks[block_index],0,frame_num);
    printf("%s\n", PoolAllocator_strerror(release_result));
  }
}



//******fnct List******


//inizializza testa della lista
void List_init(ListProcessHead* head) {
  head->first=0;
  head->last=0;
  head->size=0;
}

//trova processo nella lista dato processo
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

//trova processo nella lista dato pid processo
Process* List_find_pid(ListProcessHead* head,int pid){
  Process* aux=head->first;
  while(aux){
    if (aux->pid==pid)
      return aux;
    aux=aux->next;
  }
  return 0;
}

//inserisce processo in testa alla lista
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

//stacca processo dalla lista
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

//stampa lista processi
void List_print(ListProcessHead* head){
  Process* aux=head->first;
  while(aux){
      Process_print(aux);
      aux=aux->next;
  }
}

//libera lista processi
int List_free(ListProcessHead* head){
  Process* aux=head->first;
  while(aux){
    Process_free(aux);
  }
  return 0;
}



//******fnct SLAB******


//funzione ausiliaria
const char* PoolAllocator_strerror(PoolAllocatorResult result) {
  return PoolAllocator_strerrors[-result];
}

//alloca memoria per l'allocatore
PoolAllocator* PoolAllocator_alloc(){
  return (PoolAllocator*)malloc(sizeof(PoolAllocator));
}

//inizializza allocatore
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

//restituisce blocco di memoria, which==false: Frame ,      where==false: phymem
//                               which==true: PageTable,    where==true: swapmem
void* PoolAllocator_getBlock(MMU*mmu,bool which,bool where) {
  if(!where){
    if (mmu->phymem_allocator->first_idx==-1 || mmu->phymem_allocator->first_idx==mmu->phymem_allocator->size_max-1)
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
  }else{
    if (mmu->swapmem_allocator->first_idx==-1 || mmu->phymem_allocator->first_idx==mmu->swapmem_allocator->size_max-1)
    return 0;

    // we need to remove the first bucket from the list
    int detached_idx = mmu->swapmem_allocator->first_idx;

    //se which==0 voglio un frame altrimenti voglio una PageTable
    if(!which){
      // advance the head
      mmu->swapmem_allocator->first_idx=mmu->swapmem_allocator->free_list[mmu->swapmem_allocator->first_idx];
      --mmu->swapmem_allocator->size;
      
      mmu->swapmem_allocator->free_list[detached_idx]=DetachedIdx;
      
      //now we retrieve the pointer in the item buffer
      char* block_address=mmu->swapmem_allocator->buffer+(detached_idx*mmu->swapmem_allocator->item_size);
      mmu->swap_blocks[detached_idx]=(FrameItem*)block_address;//!
      return block_address;
    }else if (which && mmu->swapmem_allocator->size>3){
      // advance the head
      
      mmu->swapmem_allocator->first_idx=mmu->swapmem_allocator->free_list[mmu->swapmem_allocator->first_idx];
      mmu->swapmem_allocator->first_idx=mmu->swapmem_allocator->free_list[mmu->swapmem_allocator->first_idx];
      --mmu->swapmem_allocator->size;
      --mmu->swapmem_allocator->size;
      mmu->swapmem_allocator->free_list[detached_idx]=DetachedIdx;
      mmu->swapmem_allocator->free_list[detached_idx+1]=DetachedIdx;
      
      //now we retrieve the pointer in the item buffer
      char* block_address=mmu->swapmem_allocator->buffer+(detached_idx*mmu->swapmem_allocator->item_size);

      mmu->swap_blocks[detached_idx]=(PageTable*)block_address;//!
      return block_address;
    }else{
      return 0;
    }
  }
}

//rilascia un blocco  which==false: Frame
//                    which==true: PageTable
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

//stampa PoolAllocator
void PoolAllocator_PrintInfo(PoolAllocator* a){
  printf("item_size:%d\n num_free_block:%d\n buf_addr:%p\n buffer_size(Bytes):%d\n free_list addr:%p\n max_size:%d\n first_bucket_idx:%d\n ",
          a->item_size,a->size,a->buffer,a->buffer_size,a->free_list,a->size_max,a->first_idx);
}



//******fnct swap******

//trova Frame che è stato usato per ultimo con l'algoritmo di second chance partendo a ciclare dall'elemento sdopo l'ultimo cambiato
aux_struct FindVictim(Process*curr){
  aux_struct ret;
  ret.index=255;
  ret.pt_index=4096;

  int count=0;
  while(count<3){
    int i=(curr->last_changed_index+1)%PENTRY_NUM;
    int countin=0;
    while (countin<PENTRY_NUM){
      //se trovo un frame entro 
      if ((curr->pt->pe[i].flags&VALID_MASK)==Valid && !((curr->pt->pe[i].flags&UNSWAPPABLE_MASK)==Unswappable)){

        //frame è sia scritto che letto -> letto
        if (((curr->pt->pe[i].flags&WRITE_MASK)==Write) && ((curr->pt->pe[i].flags&READ_MASK)==Read)){
          curr->pt->pe[i].flags&=READ_MASK;
          curr->pt->pe[i].flags|=VALID_MASK;

        //frame è solo scritto -> prendibile al prossimo giro 
        }else if(((curr->pt->pe[i].flags&WRITE_MASK)==Write) && !((curr->pt->pe[i].flags&READ_MASK)==Read)){
          curr->pt->pe[i].flags&=VALID_MASK;

        //frame è solo letto -> prendibile al prossimo giro 
        }else if(!((curr->pt->pe[i].flags&WRITE_MASK)==Write) && ((curr->pt->pe[i].flags&READ_MASK)==Read)){
          curr->pt->pe[i].flags&=VALID_MASK;

        //frame è solo valido, ho trovato la mia vittima
        }else{
          printf("il frame corrente ha flags: %d\n", curr->pt->pe[i].flags);
          printf("la PageEntry %d sta per essere swappata\n",i);
          curr->pt->pe[i].flags=Swapped;
          ret.index=curr->pt->pe[i].frame_number;
          ret.pt_index=i;
          return ret;
        }
      }
      i=(i+1)%PENTRY_NUM;
      countin++;
    }
    count++;
  }
  //dopo tre giri se non ho trovato vittime vuol dire che il processo non ha Frame validi
  printf("Nessun frame è valido\n");
  return ret;
}

//caccia un processo dalla phy mem e lo sposta nella swapmem
int SwapOut_Frame(MMU* mmu,aux_struct indexes){
  int ret=0;
  printf("Rilasciato Frame: %d\n",indexes.index);
  What_print(mmu->phy_blocks_what[indexes.index]);
  FrameEntry_print((FrameItem*)mmu->phy_blocks[indexes.index]);

  int frame_num=mmu->swapmem_allocator->first_idx;
  FrameItem* victim_frame=(FrameItem*)mmu->phy_blocks[indexes.index];
  FrameItem* swap_frame=(FrameItem*)PoolAllocator_getBlock(mmu,false,true);
  if(swap_frame){
    memcpy(swap_frame, victim_frame, sizeof(FrameItem));
    mmu->swap_blocks_what[frame_num].what=FrameItem_;
    mmu->swap_blocks_what[frame_num].from_where=indexes.pt_index;
    printf("il frame %d della swapmem era la entry %d della pt del processo %d\n",frame_num,indexes.pt_index,mmu->curr_proc->pid);
    Frame_release(mmu,indexes.index,false);
    FrameEntry_print((FrameItem*)mmu->swap_blocks[frame_num]);
    return ret=1;
  }else{
    printf("LA SWAPMEM E' PIENA\n");
    ret=0;
  }
};

//trova in swapmem l'indirizzo logico richiesto
uint16_t FindAddress(MMU*mmu,uint16_t logicaladdress){
  uint16_t ret=0;
  for(int i=0;i<PENTRY_NUM;i++){
    if(mmu->swap_blocks_what[i].from_where==logicaladdress){
      return ret=i;
    }
  }
  return ret;
};


//sugar
void What_print(what_Flag what){
  switch (what){
  case 0x1:
    printf("Frame libero\n");
    break;
  case 0x2:
    printf("Frame entry\n");
    break;
  case 0x3:
    printf("PageTable\n");
    break;
  default:
    printf("Non identificato\n");
  }
}

LogicalAddress FormLogAddr(uint16_t offset,uint16_t pt_index){
  LogicalAddress l_a;
  l_a.offset=offset;
  l_a.pt_index=pt_index;
  return l_a;
}
