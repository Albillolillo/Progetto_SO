/*
#include "linked_list.h"

//**Costants**

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
#define PAGE_NUM (1<<FRAME_INDEX_BITS)//numero frame nella memoria fisica

#define PHYMEM_MAX_SIZE (1<<(PHYADDR_BITS)) //dimensine massima memoria fisica (1MB)
#define LOGMEM_MAX_SIZE (1<<(LOGADDR_BITS)) //dimensine massima memoria virtuale (16MB)
#define SWAPMEM_MAX_SIZE (1<<(SWAPADDR_BITS)) //dimensine massima memoria virtuale (16MB)

#define FRAME_INFO_SIZE ((1<<FRAME_OFFSET_BITS)-sizeof(ListItem)-(2*sizeof(int)))
// object size=4K
# define ITEM_SIZE sizeof(FrameItem)

// 256 blocks
#define num_items_phymem (1<<(FRAME_INDEX_BITS))
#define num_items_swapmem (1<<(SWAP_INDEX_BITS))

// buffer should contain also bookkeeping information
#define buffer_size_phymem num_items_phymem*(item_size+sizeof(int))
#define buffer_size_swapmem num_items_swapmem*(item_size+sizeof(int))

//numero massimo processi
#define MAX_NUM_PROCS 10

spostati dentro my_mmu.h*/
