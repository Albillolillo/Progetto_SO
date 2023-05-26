#pragma once



typedef struct ListProcessHead {
  Process* first;
  Process* last;
  int size;
} ListProcessHead;

void List_init(ListProcessHead* head);
Process* List_find(ListProcessHead* head, Process* item);
Process* List_insert(ListProcessHead* head, Process* previous, Process* item);
Process* List_detach(ListProcessHead* head, Process* item);
