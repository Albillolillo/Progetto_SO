#include "linked_list.h" 
#include <assert.h>

void List_init(ListHead* head) {
  head->first=0;
  head->last=0;
  head->size=0;
}

ListItem* List_find(ListHead* head, Process* item) {
  // linear scanning of list
  Process* aux=head->first;
  while(aux){
    if (aux==item)
      return item;
    aux=aux->next;
  }
  return 0;
}

ListItem* List_insert(ListHead* head, Process* prev, Process* item) {
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

ListItem* List_detach(ListHead* head, Process* item) {

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
void List_print(ListHead* head){
    Process* aux=head->first;
  while(aux){
      Process_print(aux);
      aux=aux->next;
  }
}