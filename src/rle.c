#include "rle.h"
#include <stdio.h>

void write_pair(FILE* out, int a, int count){
  //TODO: handle case where count > 255
  fputc(a, out);
  fputc(count, out);
}

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

