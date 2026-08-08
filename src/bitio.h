#ifndef BITIO_H
#define BITIO_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    FILE *file;
    uint8_t buffer;
    int bit_count;
} BitWriter;

typedef struct {
    FILE *file;
    uint8_t buffer;
    int bit_count;
} BitReader;

void bitwriter_init(BitWriter *bw, FILE *file);
void bitwriter_write_bit(BitWriter *bw, uint8_t bit);
void bitwriter_flush(BitWriter *bw);

void bitreader_init(BitReader *br, FILE *file);
uint8_t bitreader_read_bit(BitReader *br);


#endif
