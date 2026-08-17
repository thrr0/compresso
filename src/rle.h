#include <stdio.h>
#include <stdint.h>

void write_pair(FILE* out, int a, int count);
void rle_encode(FILE* in, FILE* out);
void rle_decode(FILE* in, FILE* out);
