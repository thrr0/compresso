#include <stdio.h>
#include <assert.h>
#include "bitio.h"

int main(void) {
    // uint_fast8_t bits[] = {1, 0, 1, 1, 0, 0, 0, 1};
    // int n = 8;
    //
    // FILE *f = fopen("test.bin", "wb");
    // BitWriter bw;
    // bitwriter_init(&bw, f);
    // for (int i = 0; i < n; i++) {
    //     bitwriter_write_bit(&bw, bits[i]);
    // }
    // bitwriter_flush(&bw);
    // fclose(f);
    //
    // f = fopen("test.bin", "rb");
    // BitReader br;
    // bitreader_init(&br, f);
    // for (int i = 0; i < n; i++) {
    //     uint_fast8_t bit = bitreader_read_bit(&br);
    //     assert(bit == bits[i]);
    // }
    // fclose(f);
    //
    // printf("roundtrip ok\n");
    return 0;
}
