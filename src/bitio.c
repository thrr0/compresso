#include "bitio.h"

void bitwriter_init(BitWriter *bw, FILE *file) {
    bw->file = file;
    bw->buffer = 0;
    bw->bit_count = 0;
}
