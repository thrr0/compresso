#include "rle.h"
#include <stdio.h>

//Writes one (symbol, count) pair. count must be in [1,255]
//Otherwise fputs truncates to 8 bits
static void write_pair(FILE* out, int a, int count){
  fputc(a, out);
  fputc(count, out);
}

//Run-Length Encoding 'in' into 'out'
void rle_encode(FILE *in, FILE *out){
  int byte = fgetc(in);
  int count = 1;

  while(byte != EOF){
    int next_byte = fgetc(in);

    if(byte == next_byte && count<255) count+=1;
    else {
      write_pair(out, byte, count);
      count = 1;
      byte = next_byte;
    }

  }
}

//Reverses rle_encode
void rle_decode(FILE* in, FILE* out){
  int byte = fgetc(in);
  int count = fgetc(in);

  while(byte != EOF){
    for(int i = 0; i < count; i++){
      fputc(byte, out);
    }

    byte = fgetc(in);
    count = fgetc(in);
  }
}

