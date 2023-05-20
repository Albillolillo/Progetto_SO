#pragma once
#include <stdint.h>
#include "constants_real.h"

//**Structs**

typedef enum {
    Valid=0x1,
    Read=0x2,
    Write=0x4,
    Unswappable=0x8
} PTFlags;

typedef enum {
    PHYMEM=0x1,
    SWAPMEM=0x2
} PTWhere;

typedef struct LogicalAddress{
    uint32_t pt_index: PT_INDEX_BITS;
    uint32_t offset: FRAME_OFFSET_BITS;
} LogicalAddress;

typedef struct PhysicalAddress{
    uint32_t frame_index: FRAME_INDEX_BITS;
    uint32_t offset: FRAME_OFFSET_BITS;
} PhysicalAddress;

typedef struct PageEntry {
  uint32_t frame_number: FRAME_INDEX_BITS;
  uint32_t flags: PT_FLAGSBITS;
} PageEntry;

typedef struct PageTable {
    int owner;
    PhysicalAddress mem_addr;
    PTWhere current_location_flag;
    PageEntry pt[PENTRY_NUM];
} PageTable;

typedef struct MMU {
    PageTable current_pt;
    uint32_t pt_location:PHYADDR_BITS;
} MMU;

typedef struct BitsMap{
    char cursor;
    int bits_map[8]; //8 (2^3) int = 32 (2^5) bytes = 256 (2^8) bits
}BitsMap;

//**Functions**

//dato indirizzo logico ritorna inirizzo fisico
PhysicalAddress getLinearAddress(MMU* mmu, LogicalAddress logical_address);

//scrive byte a indirizzo logico specificato
MMU_writeByte(MMU* mmu,LogicalAddress pos, char c);

//legge byte a indirizzo logico specificato
char* MMU_readByte(MMU* mmu,LogicalAddress pos );

//lancia exception se indirizzo richiesto non è valido 
MMU_exception(MMU* mmu, int pos);

