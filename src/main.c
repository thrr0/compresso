#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "bitio.h"
#include "rle.h"

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

    FILE *in = fopen("tests/rle_test.txt", "rb");
    if(in == NULL){
        perror("fopen failed");
    }
    FILE *compressed = fopen("tests/rle_test.rle", "wb");
    rle_encode(in, compressed);
    fclose(in);
    fclose(compressed);

    FILE *comp_in = fopen("tests/rle_test.rle", "rb");
    FILE *out = fopen("tests/rle_test.decoded.txt", "wb");
    rle_decode(comp_in, out);
    fclose(comp_in);
    fclose(out);


    return EXIT_SUCCESS;
}
