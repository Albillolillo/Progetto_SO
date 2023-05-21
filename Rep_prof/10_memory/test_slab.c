#include <stdio.h>
#include "slab_allocator.c"
#include "constants_real.h"
#include "frame_item.c"

// object size=4K
# define item_size sizeof(FrameItem)

// 256 blocks
#define num_items (1<<(FRAME_INDEX_BITS))

// buffer should contain also bookkeeping information
#define buffer_size num_items*(item_size+sizeof(int))

// we allocate buffer in .bss
char buffer[buffer_size];

PoolAllocator allocator;

int main(int argc, char** argv) {
  printf("initializing... ");
  PoolAllocatorResult init_result=PoolAllocator_init(&allocator,
						     item_size,
						     num_items,
						     buffer,
						     buffer_size);
  printf("%s\n",PoolAllocator_strerror(init_result));

  // we allocate_all memory, and a bit more
  
  //void* blocks[num_items];
  FrameItem* blocks[num_items];
  for (int i=0; i<num_items; ++i){
    FrameItem* frame=FrameItem_alloc();
    frame=(FrameItem*)PoolAllocator_getBlock(&allocator);
    FrameItem_init(frame,i,i);
    //void*block=PoolAllocator_getBlock(&allocator);


    blocks[i]=frame;
    printf("allocation %d, block %p, size%d, pid%d, frame_num%d, buffer_size%d \n", i, frame, allocator.size, frame->pid, frame->frame_num, sizeof(frame->info));  
  }
    
  // we release all memory
  for (int i=0; i<num_items; ++i){
    FrameItem* block=blocks[i];
    if (block){
      printf("releasing... idx: %d, block %p, free %d ... ",
	     i, block, allocator.size);
      PoolAllocatorResult release_result=PoolAllocator_releaseBlock(&allocator, block);
      printf("%s\n", PoolAllocator_strerror(release_result));
    }
  }
/*
  // we release all memory again (should get a bunch of errors)
  for (int i=0; i<num_items+10; ++i){
    FrameItem* block=blocks[i];
    if (block){
      printf("releasing... idx: %d, block %p, free %d ... ",
	     i, block, allocator.size);
      PoolAllocatorResult release_result=PoolAllocator_releaseBlock(&allocator, block);
      printf("%s\n", PoolAllocator_strerror(release_result));
    }
  }
  
  // we allocate half of the memory, and release it in reverse order
  for (int i=0; i<num_items-5; ++i){
    FrameItem* block=(FrameItem*)PoolAllocator_getBlock(&allocator);
    blocks[i]=block;
    printf("allocation %d, block %p, size%d\n", i, block, allocator.size);  
  }

  for (int i=num_items-1; i>=0; --i){
    FrameItem* block=blocks[i];
    if (block){
      printf("releasing... idx: %d, block %p, free %d ... ",
	     i, block, allocator.size);
      PoolAllocatorResult release_result=PoolAllocator_releaseBlock(&allocator, block);
      printf("%s\n", PoolAllocator_strerror(release_result));
    }
  }

  // we allocate all  memory,
  // and release only even blocks, in reverse order
  // release odd blocks in reverse order
  for (int i=0; i<num_items; ++i){
    FrameItem* block=(FrameItem*)PoolAllocator_getBlock(&allocator);
    blocks[i]=block;
    printf("allocation %d, block %p, size%d\n", i, block, allocator.size);  
  }

  for (int i=num_items-1; i>=0; i-=2){
    FrameItem* block=blocks[i];
    if (block){
      printf("releasing... idx: %d, block %p, free %d ... ",
	     i, block, allocator.size);
      PoolAllocatorResult release_result=PoolAllocator_releaseBlock(&allocator, block);
      printf("%s\n", PoolAllocator_strerror(release_result));
    }
  }

  for (int i=num_items-2; i>=0; i-=2){
    FrameItem* block=blocks[i];
    if (block){
      printf("releasing... idx: %d, block %p, free %d ... ",
	     i, block, allocator.size);
      PoolAllocatorResult release_result=PoolAllocator_releaseBlock(&allocator, block);
      printf("%s\n", PoolAllocator_strerror(release_result));
    }
  }
*/
  
}
