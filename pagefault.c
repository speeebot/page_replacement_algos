#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct page{
  int data;
  int last_used; //keep track of when page was last used, for LRU
} page_t;

typedef struct frame {
  page_t *page; //frame contains n pages (between 1 and 50)
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

int is_empty(node_t **head) {
  if(head == NULL)
    return 1;
  else return 0;
}

void print_final_state() {
  int i;
  for(i = 0; i < memory_size; i++) {
    printf("Frame %d has page %d\n", i, 49);
  }
}

void first_in_first_out() {
  //place refs in queue
  int i;
  page_t *p;

  printf("ref count: %d\n", ref_count);
  for(i = 0; i < ref_count; i++) {
    p = malloc(sizeof(p));
    p->data = page_refs[i];
    printf("p->data: %d\n", p->data);
    enqueue(&page_queue, p);
  }

  //initialize an array of size n frames, each of which contain a page
  page_frames = malloc(memory_size * sizeof(page_frames));

  //FIFO scheme
  page_t *temp;
  for(;;) {
    int cur_ref = page_queue->page->data;
    for(i = 0; i < memory_size; i++) {
      if(page_frames[i].page == NULL) {
        page_frames[i].page = cur_ref;
        temp = dequeue(&page_queue);
        continue;
      }
    }
    
    
    
    if(temp == NULL) 
      break;
  }
  

  print_final_state();
  free(p);
  free(page_frames);
}

void least_recently_used() {


  print_final_state();
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
  void* temp;
  page_refs = malloc(sizeof(page_refs));
  while(!feof(fp)) {
    ref_count++;
    if((temp = realloc(page_refs, ref_count * sizeof(page_refs)))) {
      page_refs = temp;
      fscanf(fp, "%d", &page_refs[ref_count-1]);
    }
    else {
      printf("realloc error\n");
      exit(1);
    }
  }

  //validate and run the scheme which the user selected
  if(!strcmp(argv[1], "FIFO")) {
    printf("FIFO, page refs: ");
    for(i = 0; i < ref_count; i++) {
      printf("%d ", page_refs[i]);
    }
    printf(", memsize: %d, \n", memory_size);
    first_in_first_out();
    exit(0);
  }
  else if(!strcmp(argv[1], "LRU")) {
    printf("LRU, page refs: %d, memsize: %d\n", ref_count, memory_size);
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
