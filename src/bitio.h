#ifndef BITIO_H
#define BITIO_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    FILE *file;
    uint8_t buffer;
    int bit_count;
} BitWriter;

void bitwriter_init(BitWriter *bw, FILE *file);

#endif
