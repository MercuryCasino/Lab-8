// list/list.c
// 
// Implementation for linked list.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

list_t *list_alloc() { 
  list_t* list = (list_t*)malloc(sizeof(list_t));
  list->head = NULL;
  return list; 
}

node_t *node_alloc(block_t *blk) {   
  node_t* node = malloc(sizeof(node_t));
  node->next = NULL;
  node->blk = blk;
  return node; 
}

void list_free(list_t *l) {
    if (l == NULL) return;
    
    node_t *current = l->head;
    while (current != NULL) {
        node_t *next_node = current->next;
        if (current->blk != NULL) {
            free(current->blk); // Free the block data
        }
        node_free(current);     // Free the node itself
        current = next_node;
    }
    free(l); // Free the list wrapper
}

void node_free(node_t *node){
  free(node);
  return;
}

void list_print(list_t *l) {
  node_t *current = l->head;
  block_t *b;
  
  if (current == NULL){
    printf("list is empty\n");
  }
  while (current != NULL){
    b = current->blk;
    printf("PID=%d START:%d END:%d", b->pid, b->start, b->end);
    current = current->next;
  }
}

int list_length(list_t *l) { 
  node_t *current = l->head;
  int i = 0;
  while (current != NULL){
    i++;
    current = current->next;
  }
  
  return i; 
}

void list_add_to_back(list_t *l, block_t *blk){  
  node_t* newNode = node_alloc(blk);
  newNode->next = NULL;
  if(l->head == NULL){
    l->head = newNode;
  }
  else{
    node_t *current = l->head;
    while(current->next != NULL){
      current = current->next;
    }
    current->next = newNode;
  }
}

void list_add_to_front(list_t *l, block_t *blk){  
  node_t* newNode = node_alloc(blk);
 
  newNode->next = l->head;
  l->head = newNode;
}

void list_add_at_index(list_t *l, block_t *blk, int index) {
    node_t *newNode = node_alloc(blk);
    
    if (index == 0) {
        newNode->next = l->head; // Previously was l->head->next, which leaked memory
        l->head = newNode;
        return;
    }
    
    int i = 0;
    node_t *current = l->head;
    while (i < index - 1 && current != NULL) {
        current = current->next;
        i++;
    }
    
    if (current != NULL) {
        newNode->next = current->next;
        current->next = newNode;
    } else {
        free(newNode); // Prevent memory leak if index is out of bounds
    }
}

void list_add_ascending_by_address(list_t *l, block_t *newblk){
  
   /*
   * 1. Insert newblk into list l in ascending order based on the START address of the block.
   * 
   *    node_t *c = l.head;
   *    Insert newblk After Current Node if:   newblk->start > c->start
   */

   node_t *newNode = node_alloc(newblk);

   if (l->head == NULL) || l->head->blk->start > newblk->start) {
     newNode->next = l->head;
     l->head = newNode;
     return;
   }
   
   node_t *current = l->head;
   while (current->next != NULL && current->next->blk->start <= newblk->start){
    current = current->next;
   }

   newNode->next = current->next;
   current->next = newNode;
}

void list_add_ascending_by_blocksize(list_t *l, block_t *newblk) {
    node_t *newNode = node_alloc(newblk);
    int newblk_size = (newblk->end - newblk->start) + 1;
    
    // Check if list is empty
    if (l->head == NULL) {
        newNode->next = l->head;
        l->head = newNode;
        return;
    }
    
    // Check if new block should be the new head
    int head_size = (l->head->blk->end - l->head->blk->start) + 1;
    if (newblk_size < head_size) {
        newNode->next = l->head;
        l->head = newNode;
        return;
    }
    
    node_t *current = l->head;
    while (current->next != NULL) {
        int next_size = (current->next->blk->end - current->next->blk->start) + 1;
        if (newblk_size < next_size) {
            break;
        }
        current = current->next;
    }
    
    newNode->next = current->next;
    current->next = newNode;
}

void list_add_descending_by_blocksize(list_t *l, block_t *blk){
  node_t *current;
  node_t *prev;
  node_t *newNode = node_alloc(blk);
  int newblk_size = blk->end - blk->start;
  int curblk_size;
  
  if(l->head == NULL){
    l->head = newNode;
  }
  else{
    prev = current = l->head;
    
    curblk_size = current->blk->end - current->blk->start + 1;
    
    if(current->next == NULL) {  //only one node in list
       if(newblk_size >= curblk_size) {  // place in front of current node
          newNode->next = l->head;
          l->head = newNode;
       }
       else {   // place behind current node
          current->next = newNode;
          newNode->next = NULL;
       }
    }
    else {  // two or more nodes in list
      
       if(newblk_size >= curblk_size) {  // place in front of current node
          newNode->next = l->head;
          l->head = newNode;
       }
       else {
      
          while(current != NULL && newblk_size <= curblk_size) {
               prev = current;
               current = current->next;
               
               if(current != NULL)  // the last one in the list
                     curblk_size = current->blk->end - current->blk->start;
          }
          prev->next = newNode;
          newNode->next = current;
       }
    }
  }
}

void list_coalese_nodes(list_t *l){ 
  if (l->head == NULL) return;
  
  node_t *current = l->head;

  while (current != NULL && current->next != NULL) {
    /* If the current block and the next block are physically adjacent */
    if (current->blk->end + 1 == current->next->blk->start) {
      /* Expand the end of the current block to encompass the next block */
      current->blk->end = current->next->blk->end;
      
      /* Skip over the next node and free it */
      node_t *temp = current->next;
      current->next = temp->next;
      node_free(temp);
      
      /* Do not advance 'current' yet; the newly expanded block might be adjacent to the *new* next block */
    } else {
      /* Advance only if no merge occurred */
      current = current->next;
    }
  }
}

block_t* list_remove_from_back(list_t *l) {
    block_t *value = NULL;
    
    if (l->head != NULL) {
        node_t *current = l->head;
        
        if (current->next == NULL) { 
            // Only one node in the list
            value = current->blk;
            l->head = NULL; // Previously was l->head->next = NULL
            node_free(current);
        } else {
            // Traverse to the second-to-last node
            while (current->next->next != NULL) {
                current = current->next;
            }
            value = current->next->blk; // Get the block from the last node
            node_free(current->next);   // Free the last node
            current->next = NULL;       // Sever the tie
        }
    }
    return value;
}

block_t* list_get_from_front(list_t *l) {
  block_t *value = NULL;
  if(l->head == NULL){
    return value;
  }
  else{
    node_t *current = l->head;
    value = current->blk;
  }
  return value; 
}


block_t* list_remove_from_front(list_t *l) { 
  block_t *value = NULL;
  if(l->head == NULL){
    return value;
  }
  else{
    node_t *current = l->head;
    value = current->blk;
    l->head = l->head->next;
    node_free(current);
  }
  return value; 
}

block_t* list_remove_at_index(list_t *l, int index) { 
  int i;
  block_t* value = NULL;
  
  bool found = false;
  
  if(l->head == NULL){
    return value;
  }
  else if (index == 0){
    return list_remove_from_front(l);
  }
  else if (index > 0){
    node_t *current = l->head;
    node_t *prev = current;
    
    i = 0;
    while(current != NULL && !found){
      if(i == index)
          found = true;
      else {
         prev = current;
         current = current->next;
         i++;
      }
    }
    
    if(found) {
      value = current->blk; 
      prev->next = current->next;
      node_free(current);
    }
  }
  return value; 
}

bool compareBlks(block_t* a, block_t *b) {
  
  if(a->pid == b->pid && a->start == b->start && a->end == b->end)
     return true;
  
  return false;
}

bool compareSize(int a, block_t *b) {  // greater or equal
  
  if(a <= (b->end - b->start) + 1)
     return true;
  
  return false;
}

bool comparePid(int a, block_t *b) {
  
  if(a == b->pid)
     return true;
  
  return false;
}


bool list_is_in(list_t *l, block_t* value) { 
  node_t *current = l->head;
  while(current != NULL){
    if(compareBlks(value, current->blk)){
      return true;
    }
    current = current->next;
  }
return false; 
}

block_t* list_get_elem_at(list_t *l, int index) { 
 int i;
  block_t* value = NULL;
  if(l->head == NULL){
    return value;
  }
  else if (index == 0){
    return list_get_from_front(l);
  }
  else if (index > 0){
    node_t *current = l->head;
    
    i = 0;
    while(current != NULL){
      if(i == index)
          return(current->blk);
      else {
         current = current->next;
         i++;
      }
    }
  }
  return value; 
}

int list_get_index_of(list_t *l, block_t* value){
 int i = 0;
 node_t *current = l->head;
 if(l->head == NULL){
    return -1;
  }
  
  while (current != NULL){
   if (compareBlks(value,current->blk)){
     return i;
    }
    current = current->next;
    i++;
  }
  return -1; 
}

/* Checks to see if block of Size or greater exists in the list. */
bool list_is_in_by_size(list_t *l, int Size){ 
  node_t *current = l->head;
  while(current != NULL){
    if(compareSize(Size, current->blk)){
      return true;
    }
    current = current->next;
  }
return false; 
}

/* Checks to see if pid of block exists in the list. */
bool list_is_in_by_pid(list_t *l, int pid){ 
  

  node_t *current = l->head;
  while (current != NULL) {
    if (comparePid(pid, current->blk)){
      return true;
    }
    current = current->next;
  }
  return false;
}

/* Returns the index at which the given block of Size or greater appears. */
int list_get_index_of_by_Size(list_t *l, int Size){
 int i = 0;
 node_t *current = l->head;
 if(l->head == NULL){
    return -1;
  }
  
  while (current != NULL){
   if (compareSize(Size,current->blk)){
     return i;
    }
    current = current->next;
    i++;
  }

  return -1; 
}
                   
/* Returns the index at which the pid appears. */
int list_get_index_of_by_Pid(list_t *l, int pid){
 int i = 0;
 node_t *current = l->head;
 if(l->head == NULL){
    return -1;
  }
  
  while (current != NULL){
   if (comparePid(pid,current->blk)){
     return i;
    }
    current = current->next;
    i++;
  }

  return -1; 
}