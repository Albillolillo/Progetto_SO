#pragma once
#include <stdint.h>
#include <stdbool.h>



//************Costants************

//BITS
#define FRAME_OFFSET_BITS 12 //bits offset comune a ind.logico e ind.fisico
#define PT_INDEX_BITS 12 //bits indice della page table
#define LOGADDR_BITS (PT_INDEX_BITS+FRAME_OFFSET_BITS) //bits indirizzo logico è la concatenazione di page index e frame offset

#define PT_FLAGSBITS 8 //bits flags necessari per la gestione della page table
#define FRAME_INDEX_BITS 8 //bits indice del frame nella memoria fisica
#define PT_ENTRY_BITS (PT_FLAGSBITS+FRAME_INDEX_BITS) //bits entry della page table è la concatenazione di flags e frame index

#define PHYADDR_BITS (FRAME_INDEX_BITS+FRAME_OFFSET_BITS) //bits indirizzo fisico è la concatenazione di frame index e frame offset

#define SWAP_INDEX_BITS 12 //bits indice della swapmem
#define SWAPADDR_BITS (SWAP_INDEX_BITS+FRAME_OFFSET_BITS)//bits indirizzo swap è la concatenazione di swap index e frame offset


//MEM_SIZE
#define PENTRY_NUM ((1<<(PT_INDEX_BITS))- 3*sizeof(int)) //numero entry page table 
#define FRAME_NUM (1<<FRAME_INDEX_BITS)//numero frame nella memoria fisica

#define PHYMEM_MAX_SIZE (1<<(PHYADDR_BITS)) //dimensine massima memoria fisica (1MB)
#define LOGMEM_MAX_SIZE (1<<(LOGADDR_BITS)) //dimensine massima memoria virtuale (16MB)
#define SWAPMEM_MAX_SIZE (1<<(SWAPADDR_BITS)) //dimensine massima memoria virtuale (16MB)

#define FRAME_INFO_SIZE ((1<<FRAME_OFFSET_BITS)-(2*sizeof(int)))
// object size=4K
# define ITEM_SIZE sizeof(FrameItem)

// 256 blocks
#define NUM_ITEMS_PHYMEM (1<<(FRAME_INDEX_BITS))
#define NUM_ITEMS_SWAPMEM (1<<(SWAP_INDEX_BITS))

// buffer should contain also bookkeeping information
#define BUFFER_SIZE_PHYMEM (NUM_ITEMS_PHYMEM*(ITEM_SIZE+sizeof(int)))+ITEM_SIZE
#define BUFFER_SIZE_SWAPMEM (NUM_ITEMS_SWAPMEM*(ITEM_SIZE+sizeof(int)))+ITEM_SIZE

//numero massimo processi
#define MAX_NUM_PROCS 100

//MASK
#define VALID_MASK (1)
#define UNSWAPPABLE_MASK (1<<1)
#define READ_MASK (1<<2)
#define WRITE_MASK (1<<3)
#define SWAPPED_MASK (1<<4)

//Altro
#define NOT_A_PROCESS -1;



//************Structs************

//indirizzo logico
typedef struct LogicalAddress{
    uint16_t pt_index: PT_INDEX_BITS;
    uint16_t offset: FRAME_OFFSET_BITS;
} LogicalAddress;


//indirizzo fisico
typedef struct PhysicalAddress{
    uint8_t frame_index: FRAME_INDEX_BITS;
    uint16_t offset: FRAME_OFFSET_BITS;
} PhysicalAddress;


//indirizzo swapmem  ****NON UTILIZZATA****
typedef struct SwapAddress{
    uint16_t offset: FRAME_OFFSET_BITS;
    uint16_t swap_index: SWAP_INDEX_BITS;
} SwapAddress;


//struct FrameItem
typedef struct FrameItem{
    char info[FRAME_INFO_SIZE];
    int pid;
    int frame_num;
}FrameItem;


//struct PageEntry
typedef struct PageEntry {
  uint8_t frame_number: FRAME_INDEX_BITS;
  uint8_t flags: PT_FLAGSBITS;
} PageEntry;


//struct PageTable
typedef struct PageTable {
    int pid;
    PhysicalAddress phymem_addr;
    PageEntry pe[PENTRY_NUM];
} PageTable;


//flags page_table
typedef enum {
    Free=0x0,
    Valid=0x1,
    Unswappable=0x2,
    Read=0x4,
    Write=0x8,
    Swapped=0x10  
} PTFlags;


//struct processo
typedef struct Process{
    struct Process* next;
    struct Process* prev;
    int pid;
    PageTable* pt;
    uint16_t last_changed_index;
} Process;


//struct list
typedef struct ListProcessHead {
  Process* first;
  Process* last;
  int size;
} ListProcessHead;


//structs per SLAB
typedef enum {
  Success=0x0,
  NotEnoughMemory=-1,
  UnalignedFree=-2,
  OutOfRange=-3,
  DoubleFree=-4
} PoolAllocatorResult;


typedef struct PoolAllocator{
  
  char* buffer;        //contiguous buffer managed by the system
  int*  free_list;     //list of linked objects
  int buffer_size;     //size of the buffer in bytes

  int size;            //number of free blocks
  int size_max;        //maximum number of blocks
  int item_size;       //size of a block
  
  int first_idx;       //pointer to the first bucket
} PoolAllocator;


//flag che indica cosa è stato allocato nella memoria
typedef enum {
    Free_=0x1,
    FrameItem_=0x2,
    PageTable_=0x3,
} what_Flag;


//struttura per elementi della swap_blocks_what
typedef struct what_struct {
    what_Flag what;
    uint16_t from_where;
} what_struct;


//struct memory management unit
typedef struct MMU {
    PoolAllocator* phymem_allocator;
    PoolAllocator* swapmem_allocator;
    void* phy_blocks[NUM_ITEMS_PHYMEM];
    what_Flag phy_blocks_what[NUM_ITEMS_PHYMEM];
    void* swap_blocks[NUM_ITEMS_SWAPMEM];
    what_struct swap_blocks_what[NUM_ITEMS_SWAPMEM];
    ListProcessHead* MMU_processes;
    Process* curr_proc;
} MMU;


// tipo di ritorno del FindVictim
typedef struct aux_struct {
    uint8_t index;
    uint16_t pt_index;
} aux_struct;



//************Functions************

//fnct su MMU
MMU* MMU_create(char*buffer_phymem,char*buffer_swapmem);
PhysicalAddress getPhysicalAddress(MMU* mmu, LogicalAddress logical_address);
void MMU_writeByte(MMU* mmu,LogicalAddress logical_address, char c);
char* MMU_readByte(MMU* mmu,LogicalAddress logical_address);
FrameItem* MMU_exception(MMU* mmu,LogicalAddress logical_address);
void MMU_process_update(MMU* mmu);
void MMU_print(MMU* mmu);



//fnct su FrameItem
void FrameEntry_init(FrameItem* item, int pid, uint32_t frame_num);
void FrameEntry_print(FrameItem* item);
FrameItem* FrameEntry_create(MMU* mmu);
void Frame_release(MMU*mmu,int block_index,bool where);



//fnct Processo
Process* Process_alloc();
int Process_free(Process* item);
void Process_init(Process* item, int pid,MMU* mmu);
void Process_release(Process* item,MMU* mmu);
void Process_print(Process* item);



//fnct PageTable
void PageTable_init(PageTable* pt,MMU* mmu, uint8_t frame_num);
void PageTable_print(PageTable* pt);
PageTable* PageTable_create(MMU* mmu);
void PageTable_release(MMU*mmu,int block_index);



//fnct per list
void List_init(ListProcessHead* head);
Process* List_find(ListProcessHead* head, Process* item);
Process* List_find_pid(ListProcessHead* head,int pid);
Process* List_insert(ListProcessHead* head, Process* previous, Process* item);
Process* List_detach(ListProcessHead* head, Process* item);
void List_print(ListProcessHead* head);
int List_free(ListProcessHead* head);



//fnct per SLAB
PoolAllocator* PoolAllocator_alloc();
PoolAllocatorResult PoolAllocator_init(PoolAllocator* allocator,int item_size,int num_items,char* memory_block,int memory_size);
void* PoolAllocator_getBlock(MMU* mmu,bool which,bool where);
PoolAllocatorResult PoolAllocator_releaseBlock(PoolAllocator* a, void* block_,bool which);			
const char* PoolAllocator_strerror(PoolAllocatorResult result);
void PoolAllocator_PrintInfo(PoolAllocator* a);



//fnct per Swap
aux_struct FindVictim(Process*curr);
uint16_t FindAddress(MMU*mmu,uint16_t logicaladdress);
int SwapOut_Frame(MMU* mmu,aux_struct indexes);



//sugar
void What_print(what_Flag what);

LogicalAddress FormLogAddr(uint16_t offset,uint16_t pt_index);