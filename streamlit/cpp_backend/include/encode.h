#ifndef ENCODE_H
#define ENCODE_H

#include "types.h"
#include "common.h"
#include <cstdint>
#include <cstdio>
#include <cstring> // For strlen etc.

Status do_encoding(EncodeInfo *encInfo, const char *magic_string_arg);
Status open_files(EncodeInfo *encInfo);
Status check_capacity(EncodeInfo *encInfo);
uint get_image_size_for_bmp(FILE *fptr_image);
uint get_file_size(FILE *fptr);
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);
Status encode_magic_string(EncodeInfo *encInfo, const char *magic_string_arg);
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);
Status encode_secret_file_extn_size(uint8_t ext_size, EncodeInfo *encInfo);
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);
Status encode_secret_file_data(EncodeInfo *encInfo);
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image); // Takes const char*
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

#endif