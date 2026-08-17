#include "huffman.h"

typedef struct Node{
  uint8_t symbol;
  uint64_t frequency;
  struct Node *left;
  struct Node *right;
}Node;

typedef struct{
  uint32_t code;
  int length;
}HuffmanCode;

////////////////////////////////////////////////////////////////////

static void count_frequency(FILE *file, uint64_t freq[256]){

  for(int i = 0; i < 256; i++){
    freq[i] = 0;
  }

  int byte = fgetc(file);

  while(byte != EOF){
    freq[byte] += 1;
    byte = fgetc(file);
  }

}

static Node *create_symbol_node(uint8_t symbol, uint64_t frequency){
  Node *node = (Node*) malloc(sizeof(Node));

  node-> symbol = symbol;
  node->frequency = frequency;
  node->left = NULL;
  node->right = NULL;

  return node;
}

static Node *find_smallest_node(Node* array[], int *size){
  Node* min = array[0];
  int min_index = 0;

  for(int i = 1; i < *size; i++){
    if(array[i]->frequency<min->frequency){
      min_index = i;
      min = array[i];
    }
  }

  array[min_index] = array[*size-1];

  *size -=1;


  return min;
}

static Node *create_internal_node(uint64_t frequency, Node *left, Node *right){
  Node * node = (Node*) malloc(sizeof(Node));

  node->symbol = 0;
  node->frequency = frequency;
  node->left = left;
  node->right = right;

  return node;
}
static Node *build_huffman_tree(uint64_t freq[256]){
  Node* active[256];
  int active_count= 0;

  for(int i = 0; i < 256; i++){
    if(freq[i]>0){
      active[active_count] = create_symbol_node(i, freq[i]);
      active_count+=1;
    }
  }

  while(active_count > 1){
    Node *a = find_smallest_node(active, &active_count);
    Node *b = find_smallest_node(active, &active_count);
    
    Node *c = create_internal_node(a->frequency + b->frequency, a, b);
    active[active_count] = c;
    active_count +=1;
  }

  return active[0];
}
/////////////////////////////////////////////////////////////////

void huffman_encode(FILE *in, FILE *out){
  uint64_t freq[256];
  count_frequency(in, freq);


}


