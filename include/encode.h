#ifndef ENCODE_H
#define ENCODE_H

#include "types.h" // Contains user defined types
#include "common.h" // Contains user defined types
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */


/* Perform the encoding */
Status do_encoding(EncodeInfo *encInfo);//done almost

/* Get File pointers for i/p and o/p files */
Status open_files(EncodeInfo *encInfo);//done

/* check capacity */
Status check_capacity(EncodeInfo *encInfo);

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image);//done 

/* Get file size */
uint get_file_size(FILE *fptr);//done

/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);//done

/* Store Magic String */
Status encode_magic_string(EncodeInfo *encInfo);//done , can take a  rg from user

/* Encode secret file extenstion */
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo);//done almost , need to take arg from cli

Status encode_secret_file_extn_size(uint8_t ext_size, EncodeInfo *encInfo);//done


/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);//done

/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo);

/* Encode function, which does the real encoding */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image);

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(char data, char *image_buffer);

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);//done

void printbin8(char a);

#endif
