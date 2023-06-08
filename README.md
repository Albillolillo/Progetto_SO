# Progetto_SO
MMU Implementation (Toy)

virtual memory space: 16 MB swap file: 16MB phyisical memory space: 1MB

The page table support the following flags:
- valid (frame is owned and it is in physical mem) 
- unswappable (PageTables are unswappable) 
- read_bit (set by the mmu each time that page is read) 
- write_bit (set by the mmu each time a page is written) 
- swapped (set by the mmu each time a page is swapped)

callable functions from main (actually every function can be called but u can consider only this functions as public):

MMU* MMU_create(char*buffer_phymem,char*buffer_swapmem);
void MMU_writeByte(MMU* mmu,LogicalAddress logical_address, char c);
char* MMU_readByte(MMU* mmu,LogicalAddress logical_address);
MMU_print(mmu);

Process* Process_alloc();
int Process_free(Process* item);
void Process_init(Process* item, int pid,MMU* mmu);
void Process_release(Process* item,MMU* mmu);
void Process_print(Process* item);

Page replacement policy implement the second chance algorithm.