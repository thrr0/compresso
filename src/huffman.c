#include "huffman.h"
#include "bitio.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// left == NULL && right == NULL marks a leaf; internal nodes always
// have both children, never just one.
typedef struct Node{
  uint8_t symbol;
  uint64_t frequency;
  struct Node *left;
  struct Node *right;
}Node;

// A code's value alone isn't enough (leading zeros get lost in an
// int) -- length is what makes e.g. 0b01 distinguishable from 0b1.
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

  node->symbol = symbol;
  node->frequency = frequency;
  node->left = NULL;
  node->right = NULL;

  return node;
}

// Removes and returns the lowest-frequency node from array[0..*size-1],
// swapping the last active element into its place. Shrinks *size by 1.
static Node *find_smallest_node(Node* array[], int *size){
  Node* min = array[0];
  int min_index = 0;

  for(int i = 1; i < *size; i++){
    if(array[i]->frequency < min->frequency){
      min_index = i;
      min = array[i];
    }
  }

  array[min_index] = array[*size - 1];
  *size -= 1;

  return min;
}

static Node *create_internal_node(uint64_t frequency, Node *left, Node *right){
  Node *node = (Node*) malloc(sizeof(Node));

  node->symbol = 0; // unused on internal nodes
  node->frequency = frequency;
  node->left = left;
  node->right = right;

  return node;
}

// Builds the Huffman tree bottom-up: repeatedly merges the two
// lowest-frequency active nodes until one (the root) remains.
static Node *build_huffman_tree(uint64_t freq[256]){
  Node* active[256];
  int active_count = 0;

  for(int i = 0; i < 256; i++){
    if(freq[i] > 0){
      active[active_count] = create_symbol_node(i, freq[i]);
      active_count += 1;
    }
  }

  while(active_count > 1){
    Node *a = find_smallest_node(active, &active_count);
    Node *b = find_smallest_node(active, &active_count);

    Node *c = create_internal_node(a->frequency + b->frequency, a, b);
    active[active_count] = c;
    active_count += 1;
  }

  return active[0];
}

// Recursively walks the tree, accumulating a code as it descends
// (left = 0, right = 1), and records each leaf's final code/length.
static void build_table(Node *node, HuffmanCode table[256], uint32_t code, int length){
  if(node->left == NULL && node->right == NULL){
    int i = node->symbol;
    table[i].code = code;
    table[i].length = length;
  }else{
    build_table(node->left, table, (code << 1), length + 1);
    build_table(node->right, table, (code << 1) + 1, length + 1);
  }
}

// Header layout: original size, symbol count, then one
// (symbol, length, code) entry per symbol that actually appears.
static void write_huffman_header(FILE *out, uint64_t original_size, HuffmanCode table[256]){
  uint16_t symbol_count = 0;
  for(int i = 0; i < 256; i++){
    if(table[i].length > 0){
      symbol_count += 1;
    }
  }

  fwrite(&original_size, sizeof(uint64_t), 1, out);
  fwrite(&symbol_count, sizeof(symbol_count), 1, out);

  for(int i = 0; i < 256; i++){
    if(table[i].length > 0){
      uint8_t symbol = (uint8_t) i;
      uint8_t len = (uint8_t) table[i].length;

      fwrite(&symbol, sizeof(uint8_t), 1, out);
      fwrite(&len, sizeof(uint8_t), 1, out);
      fwrite(&table[i].code, sizeof(uint32_t), 1, out);
    }
  }
}

// Mirror of build_table: walks/creates the path a code describes,
// creating internal nodes only where they don't exist yet, and
// places the symbol on the leaf at the end of that path.
static void insert_code(Node *tree, uint8_t symbol, uint32_t code, int length){
  Node *a = tree;
  for(int i = length - 1; i >= 0; i--){
    int bit = (code >> i) & 1;

    if(bit == 0){
      if(a->left == NULL) a->left = create_internal_node(0, NULL, NULL);
      a = a->left;
    }else{
      if(a->right == NULL) a->right = create_internal_node(0, NULL, NULL);
      a = a->right;
    }
  }
  a->symbol = symbol;
}

static Node *read_huffman_header(FILE *in, uint64_t *file_size){
  uint16_t symbol_count;
  Node *tree = (Node*) malloc(sizeof(Node));
  fread(file_size, sizeof(uint64_t), 1, in);
  fread(&symbol_count, sizeof(uint16_t), 1, in);

  tree->left = NULL;
  tree->right = NULL;

  uint8_t symbol;
  uint8_t length;
  uint32_t code;

  for(int i = 0; i < symbol_count; i++){
    fread(&symbol, sizeof(uint8_t), 1, in);
    fread(&length, sizeof(uint8_t), 1, in);
    fread(&code, sizeof(uint32_t), 1, in);

    insert_code(tree, symbol, code, length);
  }
  return tree;
}

// Writes a code's bits most-significant-first (i.e. root-to-leaf
// order), independent of bitio's own LSB-first byte packing.
static void write_code(BitWriter *bw, HuffmanCode c){
  int i = c.length - 1;
  while(i >= 0){
    uint8_t bit = (c.code >> i) & 1;
    bitwriter_write_bit(bw, bit);
    i -= 1;
  }
}

static void free_tree(Node *node){
  if(node == NULL) return;
  free_tree(node->left);
  free_tree(node->right);
  free(node);
}

/////////////////////////////////////////////////////////////////

void huffman_encode(FILE *in, FILE *out){
  fseek(in, 0, SEEK_END);
  uint64_t original_size = ftell(in);
  rewind(in);

  uint64_t freq[256];
  count_frequency(in, freq);

  Node *huffmanTree = build_huffman_tree(freq);
  HuffmanCode table[256];

  for(int i = 0; i < 256; i++){
    table[i].length = 0;
  }

  if(huffmanTree->left == NULL && huffmanTree->right == NULL){
    // Single distinct symbol: no real tree to walk, force a 1-bit code.
    table[huffmanTree->symbol].code = 0;
    table[huffmanTree->symbol].length = 1;
  }else{
    build_table(huffmanTree, table, 0, 0);
  }

  free_tree(huffmanTree);

  write_huffman_header(out, original_size, table);

  rewind(in);

  BitWriter io_out;
  bitwriter_init(&io_out, out);

  int byte = fgetc(in);
  while(byte != EOF){
    write_code(&io_out, table[byte]);
    byte = fgetc(in);
  }
  bitwriter_flush(&io_out);
}

void huffman_decode(FILE *in, FILE *out){
  uint64_t file_size;
  Node *tree = read_huffman_header(in, &file_size);

  BitReader br;
  bitreader_init(&br, in);

  Node *node;
  int bit;
  for(uint64_t i = 0; i < file_size; i++){
    node = tree;
    while(node->left != NULL || node->right != NULL){
      bit = bitreader_read_bit(&br);
      if(bit == 0) node = node->left;
      else if(bit == 1) node = node->right;
    }
    fputc(node->symbol, out);
  }

  free_tree(tree);
}
