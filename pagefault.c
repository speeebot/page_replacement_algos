#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


typedef struct page{
  int data;
} page_t;

typedef struct node {
  page_t *page;
  struct node *next;
} node_t;

int *page_refs;

int memory_size = 0;
int page_fault_count = 0;


void print_final_state() {
  int i;
  for(i = 0; i < memory_size; i++) {
    printf("Frame %d has page %d\n", i, 49);
  }
}

void first_in_first_out() {
  node_t *page_queue;


  print_final_state();
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
  int ref_count = 0;
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
    printf("FIFO, page refs: %d, memsize: %d, \n", ref_count, memory_size);
    first_in_first_out();
    exit(0);
  }
  else if(!strcmp(argv[1], "LRU")) {
    printf("LRU, page refs: %d, memsize: %d, \n", ref_count, memory_size);
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
