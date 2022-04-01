//Shawn Diaz
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct page{
  int data;
} page_t;

typedef struct frame {
  page_t *page; //frame contains n pages (between 1 and 50)
  int age;
} frame_t;

typedef struct node {
  page_t *page;
  struct node *next;
} node_t;

node_t *page_queue;
frame_t *page_frames;

int *page_refs;
int ref_count = 0;

int memory_size = 0;
int page_fault_count = 0;


void enqueue(node_t **head, page_t *p) {
  node_t *new_node = malloc(sizeof(node_t));
  new_node->page = p;
  new_node->next = NULL;
  
  if(*head == NULL) {
    *head = new_node;
    return;
  }

  node_t *cur_node = *head;

  while(cur_node->next)
    cur_node = cur_node->next;

  new_node->next = cur_node->next;
  cur_node->next = new_node;
}

page_t *dequeue(node_t **head) {
  node_t *cur_node = *head;

  if(!cur_node) {
    return NULL;
  }

  page_t *p = cur_node->page;
  *head = (*head)->next;
  free(cur_node);

  return p;
}


int get_oldest_frame() {
  int i, oldest_frame = 0;
  frame_t oldest = page_frames[0];

  for(i = 1; i < memory_size; i++) {
    if(page_frames[i].age > oldest.age) {
      oldest = page_frames[i];
      oldest_frame = i;
    }
  }
  return oldest_frame;
}


void print_final_state() {
  int i; 
  printf("The number of page faults was %d\n\n", page_fault_count);
  for(i = 0; i < memory_size; i++) {
    if(page_frames[i].page != NULL)
      printf("Frame %d has page %d\n", i, page_frames[i].page->data);
    else
      printf("Frame %d has nothing\n", i);
  }
}

void first_in_first_out() {
  //place refs in queue
  int i;
  page_t *p;

  for(i = 0; i < ref_count; i++) {
    p = malloc(sizeof(p));
    p->data = page_refs[i];
    enqueue(&page_queue, p);
  }

  //initialize an array of size n frames, each of which contain a page
  page_frames = malloc(memory_size * sizeof(frame_t));

  //FIFO scheme
  page_t *temp;
  int cur_ref, iter = 0;
  for(;;) {

    if(page_queue == NULL) {
      break;
    }

    //no more refs
    if(iter == ref_count)
      break;

    cur_ref = page_queue->page->data;

    //increment age of all non-empty memory slots
    for(i = 0; i < memory_size; i++) {
      if(page_frames[i].page != NULL) {
        page_frames[i].age++;
      }
    }
    
    for(i = 0; i <= memory_size; i++) {
      //reached the end of memory, miss
      if(i == memory_size) {
        int oldest_frame = get_oldest_frame();
        page_frames[oldest_frame].page->data = cur_ref;
        page_frames[oldest_frame].age = -1;
        temp = dequeue(&page_queue);
        page_fault_count++;
        break;
      }
      //slot not empty
      if(page_frames[i].page != NULL) {
        //hit, no page fault
        if(page_frames[i].page->data == cur_ref) { 
          temp = dequeue(&page_queue);
          break;
        }
      }
      //reach empty slot, no hit, slot available
      else if(page_frames[i].page == NULL) {
        page_frames[i].page = malloc(sizeof(page_t));
        page_frames[i].page->data = cur_ref;
        temp = dequeue(&page_queue);
        page_fault_count++;
        break;
      }
    }
    iter++;
  }
  print_final_state();
  free(p);
  free(page_frames);
}

void least_recently_used() {
  //place refs in queue
  int i;
  page_t *p;

  for(i = 0; i < ref_count; i++) {
    p = malloc(sizeof(p));
    p->data = page_refs[i];
    enqueue(&page_queue, p);
  }

  //initialize an array of size n frames, each of which contain a page
  page_frames = malloc(memory_size * sizeof(frame_t));

  //LRU scheme using a counter
  page_t *temp;
  int cur_ref, iter = 0;
  for(;;) {

    if(page_queue == NULL) {
      break;
    }

    //no more refs
    if(iter == ref_count)
      break;

    cur_ref = page_queue->page->data;

    //increment age of all non-empty memory slots
    for(i = 0; i < memory_size; i++) {
      if(page_frames[i].page != NULL) {
        page_frames[i].age++;
      }
    }
    
    for(i = 0; i <= memory_size; i++) {
      //reached the end of memory, miss
      if(i == memory_size) {
        int oldest_frame = get_oldest_frame();
        page_frames[oldest_frame].page->data = cur_ref;
        page_frames[oldest_frame].age = -1;
        temp = dequeue(&page_queue);
        page_fault_count++;
        break;
      }
      //slot not empty
      if(page_frames[i].page != NULL) {
        //hit, no page fault
        if(page_frames[i].page->data == cur_ref) { 
          page_frames[i].age = -1;
          temp = dequeue(&page_queue);
          break;
        }
      }
      //reach empty slot, no hit, slot available
      else if(page_frames[i].page == NULL) {
        page_frames[i].page = malloc(sizeof(page_t));
        page_frames[i].page->data = cur_ref;
        temp = dequeue(&page_queue);
        page_fault_count++;
        break;
      }
    }
    iter++;
  }
  print_final_state();
  free(p);
  free(page_frames);
}

int main(int argc, char** argv) {
  if(argc != 4) {
    printf("Usage: ./program <scheme> <page ref file> <memory size>\n");
    exit(1);
  }

  //validate memory size argument
  int i;
  for(i = 0; i < strlen(argv[3]); i++) {
    if(isdigit(argv[3][i])) {
      continue;
    }
    else {
      printf("<memory size> needs to be a number between 1 and 50\n");
      exit(1);
    }
  }
  memory_size = atoi(argv[3]);
  if(memory_size > 50 || memory_size < 1) {
    printf("<memory size> should be between 1 and 50\n");
    exit(1);
  }

  //validate and read in page refs
  FILE *fp = fopen(argv[2], "r");
  if(fp == NULL) {
    printf("File %s does not exist\n", argv[2]);
    exit(1);
  }
  //check if file is empty
  int c = fgetc(fp);
  if (c == EOF) {
    printf("File %s is empty\n", argv[2]);
    exit(1);
  } else {
    ungetc(c, fp);
  }
  //if file contains refs, read them
  void* temp;
  page_refs = malloc(sizeof(page_refs));
  while(!feof(fp)) {
    ref_count++;
    if((temp = realloc(page_refs, ref_count * sizeof(page_refs)))) {
      page_refs = temp;
      fscanf(fp, "%d ", &page_refs[ref_count-1]);
    }
    else {
      printf("realloc error\n");
      exit(1);
    }
  }

  //validate and run the scheme which the user selected
  if(!strcmp(argv[1], "FIFO")) {
    printf("scheme: First in, first out, page refs: ");
    for(i = 0; i < ref_count; i++) {
      printf("%d ", page_refs[i]);
    }
    printf(", memsize: %d\n\n", memory_size);
    first_in_first_out();
    exit(0);
  }
  else if(!strcmp(argv[1], "LRU")) {
    printf("scheme: Least recently used, page refs: ");
    for(i = 0; i < ref_count; i++) {
      printf("%d ", page_refs[i]);
    }
    printf(", memsize: %d\n\n", memory_size);
    least_recently_used();
    exit(0);
  }
  else {
    printf("Usage: ./program <scheme> <page ref file> <memory size>\n");
    printf("Usage: <scheme> should either be FIFO or LRU\n");
    exit(1);
  }

  fclose(fp);
  free(page_refs);
  exit(0);
}
