/*#pragma once
#include <assert.h>
#include <stdio.h>
#include "my_mmu.h"
#include "linked_list.h"
#include "constants_real.h"

//this is a record of the pages list. It holds the infos in the frame and the pid for a process
typedef struct FrameItem{
  ListItem list;
  int pid;
  int frame_num;
  char info[FRAME_INFO_SIZE];
}FrameItem;


FrameItem* FrameItem_alloc();
int FrameItem_free(FrameItem* item);
void FrameItem_init(FrameItem* item, int pid, uint32_t frame_num);
void FrameItem_print(FrameItem* item);
void FrameList_print(ListHead* list);
*/