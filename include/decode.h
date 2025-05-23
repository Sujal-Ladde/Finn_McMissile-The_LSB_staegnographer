#ifndef DECODE_H
#define DECODE_H

#include "common.h"
Status open_decode_files(EncodeInfo *encInfo);
uint8_t decode_magic_size(EncodeInfo *encInfo);
Status do_decoding(EncodeInfo *encInfo);
Status extract_magic(EncodeInfo *encInfo);
uint8_t decode_file_extn_size(EncodeInfo *encInfo);
Status decode_file_extension(EncodeInfo *encInfo);
Status open_dest_file(EncodeInfo *encInfo, char *name);
int secret_data_size(EncodeInfo *encInfo);

Status decode_data_from_image(int size, FILE *fptr_stego_image, char *dest);

#endif