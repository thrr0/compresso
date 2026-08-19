#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "rle.h"
#include "huffman.h"

#define MAGIC "CMP1"
#define ALGO_RLE 0
#define ALGO_HUFFMAN 1

void copy_file(FILE *a, FILE *b);

int main(int argc, char *argv[]) {
    if(argc != 4){
        fprintf(stderr, "Arguments must be 4");
        return EXIT_FAILURE;
    }

    if(strcmp("compress", argv[1]) == 0){
        FILE *in = fopen(argv[2], "rb");
        FILE *out = fopen(argv[3], "wb");

        if(in == NULL || out == NULL){
            fprintf(stderr, "Error reading file(s)");
            return EXIT_FAILURE;
        }

        FILE *tmp_rle = tmpfile();
        FILE *tmp_huff = tmpfile();

        rle_encode(in, tmp_rle);
        rewind(in);
        huffman_encode(in, tmp_huff);

        fseek(tmp_rle, 0, SEEK_END);
        long rle_size = ftell(tmp_rle);

        fseek(tmp_huff, 0, SEEK_END);
        long huff_size = ftell(tmp_huff);

        uint8_t algo;
        FILE *winner;

        if(rle_size <= huff_size){
            algo = ALGO_RLE;
            winner = tmp_rle;
        }else{
            algo = ALGO_HUFFMAN;
            winner= tmp_huff;
        }

        fwrite(MAGIC, 1, 4, out);
        fwrite(&algo, sizeof(algo), 1, out);

        rewind(winner);
        copy_file(winner, out);

        fclose(in);
        fclose(out);
        fclose(tmp_rle);
        fclose(tmp_huff);

    }
    else if(strcmp("decompress", argv[1]) == 0){
        FILE *in = fopen(argv[2], "rb");
        FILE *out = fopen(argv[3], "wb");

        if(in == NULL || out == NULL){
            fprintf(stderr, "Error reading file(s)");
            return EXIT_FAILURE;
        }

        char magic[5];
        fread(magic, 1, 4, in);
        magic[4] = '\0';

        if(strcmp(magic, MAGIC)!= 0){
            fprintf(stderr, "Incompatible file format");
            return EXIT_FAILURE;
        }

        uint8_t algo;
        fread(&algo, sizeof(algo), 1, in);

        switch(algo){
            case ALGO_RLE:
                rle_decode(in, out);
                break;
            case ALGO_HUFFMAN:
               huffman_decode(in, out);
                break;
            default:
                fprintf(stderr, "Error parsing compression format");
                break;
        }

        fclose(in);
        fclose(out);

    }else{
        fprintf(stderr, "Invalid command.");
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}

void copy_file(FILE *a, FILE *b){
    int byte = fgetc(a);
    
    while(byte != EOF){
    fputc(byte, b);
    byte = fgetc(a);
    }
}
