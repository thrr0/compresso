#include "bitio.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void bitwriter_write_buffer(BitWriter *bw){
    // TODO: handle write errors
        fputc(bw->buffer, bw->file);

        bw->bit_count =0;
        bw->buffer = 0;
}


void bitwriter_init(BitWriter *bw, FILE *file) {
    bw->file = file;
    bw->buffer = 0;
    bw->bit_count = 0;
}

void bitwriter_write_bit(BitWriter *bw, uint_fast8_t bit){
assert(bit == 0 || bit == 1);
    if(bw->bit_count == 8){
        bitwriter_write_buffer(bw);
    }
    bit = bit << bw->bit_count;
    bw->buffer = bw->buffer | bit;
    bw->bit_count+=1;
}

void bitwriter_flush(BitWriter *bw){
    if(bw->bit_count > 0){
        bitwriter_write_buffer(bw);
    }
}

void bitreader_init(BitReader *br, FILE *file){
    br->file = file;
    br->bit_count = 8;
    br->buffer = 0;
}

uint8_t bitreader_read_bit(BitReader *br){
    if(br->bit_count == 8){
        br->buffer = fgetc(br->file);
        br->bit_count= 0;
    }

    uint8_t bit = (br->buffer >> br->bit_count) & 1;
    br->bit_count += 1;
    return bit;


}

