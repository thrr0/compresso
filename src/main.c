#include <stdio.h>
#include "bitio.h"

int main(void) {
    BitWriter bw;
    bitwriter_init(&bw, stdout);
    printf("toolchain ok, bit_count inicial: %d\n", bw.bit_count);
    return 0;
}
