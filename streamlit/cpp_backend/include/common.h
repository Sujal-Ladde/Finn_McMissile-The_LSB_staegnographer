#ifndef COMMON_H
#define COMMON_H

#include "types.h" // Contains user defined types
// #include "common.h" // Redundant self-include
#include <cstdint> // Use <cstdint> instead of <stdint.h>
#include <cstdio>  // Use <cstdio> instead of <stdio.h>
// #include <unistd.h> // unistd.h is POSIX-specific, consider alternatives if cross-platform beyond Linux/macOS is critical
#include <cstring> // Use <cstring> instead of <string.h>
#include <cstdlib> // Use <cstdlib> instead of <stdlib.h>
#include <ctime>   // Use <ctime> instead of <time.h>

// _POSIX_C_SOURCE might not be needed if not using highly specific POSIX features directly in headers
// #define _POSIX_C_SOURCE 200809L

typedef struct _EncodeInfo
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    uint image_capacity; // Calculated, not directly set by Python
    uint bits_per_pixel; // Usually 24 for BMP, can be assumed or derived

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char MAGIC_STRING[50]; // Increased size for flexibility
    uint8_t magic_size;    // Derived from MAGIC_STRING
    uint8_t ext_size;      // Derived from secret_fname
    char *ext;             // Derived from secret_fname
    long size_secret_file; // Derived from secret_fname

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    // destination file for decoded output
    char *dest_file; // For decoding output path
    FILE *fptr_dest_file;

} EncodeInfo;

// These utility functions are fine, but clear_screen_c is not for a library
// void get_current_time_string_ms(char *buffer, size_t buffer_size); // Implementation not provided
// void clear_screen_c(); // Remove for library
char *int_to_str(uint num);
uint str_to_int(const char* data); // Add const
#endif