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
#define PENTRY_NUM (1<<(PT_INDEX_BITS)) //numero entry page table 
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
#define BUFFER_SIZE_PHYMEM NUM_ITEMS_PHYMEM*(ITEM_SIZE+sizeof(int))
#define BUFFER_SIZE_SWAPMEM NUM_ITEMS_SWAPMEM*(ITEM_SIZE+sizeof(int))

//numero massimo processi
#define MAX_NUM_PROCS 10



//************Structs************

//indirizzo logico
typedef struct LogicalAddress{
    uint16_t offset: FRAME_OFFSET_BITS;
    uint16_t pt_index: PT_INDEX_BITS;
} LogicalAddress;

//indirizzo fisico
typedef struct PhysicalAddress{
    uint16_t offset: FRAME_OFFSET_BITS;
    uint8_t frame_index: FRAME_INDEX_BITS;
} PhysicalAddress;

//indirizzo swapmem
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
  uint32_t frame_number: FRAME_INDEX_BITS;
  uint32_t flags: PT_FLAGSBITS;
} PageEntry;

//struct PageTable
typedef struct PageTable {
    int pid;
    int frame_num;
    PhysicalAddress phymem_addr;
    SwapAddress swapmem_addr;
    PageEntry pe[PENTRY_NUM];
} PageTable;

//flags page_table
typedef enum {
    Valid=0x1,
    Read=0x2,
    Write=0x4,
    Unswappable=0x8
} PTFlags;



//struct processo
typedef struct Process{
    struct Process* next;
    struct Process* prev;
    int pid;
    PageTable* pt;
    bool on_disk; //false phy mem ,true swap mem
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


//struct memory management unit
typedef struct MMU {
    PoolAllocator* phymem_allocator;
    PoolAllocator* swapmem_allocator;
    ListProcessHead* MMU_processes;
    Process* curr_proc;
} MMU;








//************Functions************

//fnct su MMU

//alloca e inizzializza MMU
MMU* MMU_create(PoolAllocator* phymem_allocator,PoolAllocator* swapmem_allocator,Process* curr, ListProcessHead* processes);
//dato indirizzo logico ritorna inirizzo fisico
PhysicalAddress getPhysicalAddress(MMU* mmu, LogicalAddress logical_address);
//scrive byte a indirizzo logico specificato
void MMU_writeByte(MMU* mmu,LogicalAddress pos, char c);
//legge byte a indirizzo logico specificato
char* MMU_readByte(MMU* mmu,LogicalAddress pos);
//lancia exception se indirizzo richiesto non è valido 
void MMU_exception(MMU* mmu, int pos);
//stampa MMU
void MMU_print(MMU* mmu);

//fnct su FrameItem

//inizializza frame
void FrameEntry_init(FrameItem* item, int pid, uint32_t frame_num);
//stampa un frame
void FrameEntry_print(FrameItem* item);




//fnct Processo

//alloca mem per processo
Process* Process_alloc();
//libera processo
int Process_free(Process* item);
//inizializza processo
void Process_init(Process* item, int pid);
//stampa un processo
void Process_print(Process* item);



//fnct PageTable

//inizzializza PageTable
void PageTable_init(PageTable* pt,MMU* mmu,int pid,int frame_num);
//stampa PageTable
void PageTable_print(PageTable* pt);



//fnct per list
//
void List_init(ListProcessHead* head);
Process* List_find(ListProcessHead* head, Process* item);
Process* List_insert(ListProcessHead* head, Process* previous, Process* item);
Process* List_detach(ListProcessHead* head, Process* item);
void List_print(ListProcessHead* head);
int List_free(ListProcessHead* head);



//fnct per SLAB
PoolAllocator* PoolAllocator_alloc();

PoolAllocatorResult PoolAllocator_init(PoolAllocator* allocator,
			int item_size,
			int num_items,
			char* memory_block,
			int memory_size);

void* PoolAllocator_getBlock(PoolAllocator* allocator,bool which);

PoolAllocatorResult PoolAllocator_releaseBlock(PoolAllocator* allocator, FrameItem* block);
			
const char* PoolAllocator_strerror(PoolAllocatorResult result);

void PoolAllocator_PrintInfo(PoolAllocator* a);