#ifndef DECODE_H
#define DECODE_H

#include "common.h"

Status open_decode_files(EncodeInfo *encInfo); // Only opens stego image for reading
uint8_t decode_magic_size(EncodeInfo *encInfo); // Internal helper

// Pass magic string as parameter
Status do_decoding(EncodeInfo *encInfo, const char *magic_string_arg);

// Pass magic string for comparison
Status extract_magic(EncodeInfo *encInfo, const char *magic_string_arg);

uint8_t decode_file_extn_size(EncodeInfo *encInfo); // Internal helper
Status decode_file_extension(EncodeInfo *encInfo); // Modifies encInfo->dest_file potentially
Status open_dest_file(EncodeInfo *encInfo, const char *name); // Make name const
int secret_data_size(EncodeInfo *encInfo); // Internal helper

Status decode_data_from_image(int size, FILE *fptr_stego_image, char *dest);
Status decode_secret_data(EncodeInfo *encInfo); // Internal helper

#endif