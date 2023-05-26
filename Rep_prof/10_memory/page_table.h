#include "my_mmu.h"
#include <stdint.h>

typedef struct PageEntry {
  uint32_t frame_number: FRAME_INDEX_BITS;
  uint32_t flags: PT_FLAGSBITS;
} PageEntry;

typedef struct PageTable {
    PhysicalAddress phymem_addr;
    SwapAddress swapmem_addr;
    PageEntry pe[PENTRY_NUM];
} PageTable;

PageTable PageTable_init();
void PageTable_print();