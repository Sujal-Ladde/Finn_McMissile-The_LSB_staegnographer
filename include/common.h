#ifndef COMMON_H
#define COMMON_H


#include "types.h" // Contains user defined types
#include "common.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#define _POSIX_C_SOURCE 200809L

typedef struct _EncodeInfo
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    uint image_capacity;
    uint bits_per_pixel;

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char MAGIC_STRING[20];
    uint8_t magic_size;
    uint8_t ext_size;
    char *ext;
    long size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    // destination file
    char *dest_file;
    FILE *fptr_dest_file;

} EncodeInfo;
void get_current_time_string_ms(char *buffer, size_t buffer_size);
void clear_screen_c();
char *convert_size(uint num);

#endif
