#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Include for string functions
#include "encode.h"
#include "decode.h"
#include "types.h"

#define MAX_PATH_LENGTH 256 // Define a maximum length for file paths

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    uint img_size;
    char base__encode_ip[] = "data/encode_input/";
    char base__encode_op[] = "data/encode_output/";
    char base__decode_ip[] = "data/decode_input/";
    char base__decode_op[] = "data/decode_output/";
    char default_encode_stego[] = "data/encode_output/stego.bmp";
    char default_decode_secret[] = "data/decode_output/secret";
    if (argc == 1)
    {
        fprintf(stdout, "check --info for usage options.\n");
    } 
    else if (strcmp(argv[1], "--info") == 0)
    {
        fprintf(stdout, "\n");
        fprintf(stdout, "Usage: ./a.out [OPTION...] [SOURCE IMAGE] [MESSAGE FILE] [DESTINATION IMAGE] \n");
        fprintf(stdout, "\n");
        fprintf(stdout, "[OPTIONS...]\n");
        fprintf(stdout, "\t-e, encode\tEncodes the message in the image\n");
        fprintf(stdout, "\t-d, decode\tDecodes the encoded message from the image\n");
        fprintf(stdout, "[SOURCE IMAGE]\n");
        fprintf(stdout, "\tThe name of the source image for encoding or decoding respectively\n");
        fprintf(stdout, "[MESSAGE FILE]\n");
        fprintf(stdout, "\tIn case of encoding, The name of the message file for encoding\n");
        fprintf(stdout, "\tIn case of \033[1mDecoding \033[0mit is the destination file for the extracted message\n");
        fprintf(stdout, "\t\033[1mWARNING-- \033[0mIn case of decoding, defaults to stego.[decoded extn.] if not mentioned\n");
        fprintf(stdout, "[DESTINATION IMAGE]\n");
        fprintf(stdout, "\tThe name of the generated image with the encoded message\n");
        fprintf(stdout, "\t\033[1mWARNING-- \033[0mUsed Only for encoding, defaults to stego.bmp if not mentioned\n");
        fprintf(stdout, "\n");
    }
    else if (strcmp(argv[1], "-e") == 0)
    {

        char src_img_dest_buffer[MAX_PATH_LENGTH];
        char secret_dest_buffer[MAX_PATH_LENGTH];
        char encode_dest_buffer[MAX_PATH_LENGTH];
        char default_stego_path[MAX_PATH_LENGTH];

        if (argv[2] == NULL | argv[3] == NULL)
        {
            fprintf(stdout, "ERROR : Too few arguments, check --info for more details\n");
            return e_failure;
        }

        if (strlen(base__encode_ip) + strlen(argv[2]) < MAX_PATH_LENGTH)
        {
            strcpy(src_img_dest_buffer, base__encode_ip);
            strcat(src_img_dest_buffer, argv[2]);
            encInfo.src_image_fname = src_img_dest_buffer;
        }
        else
        {
            fprintf(stderr, "Error: Source image path too long.\n");
            return 1;
        }

        if (strlen(base__encode_ip) + strlen(argv[3]) < MAX_PATH_LENGTH)
        {
            strcpy(secret_dest_buffer, base__encode_ip);
            strcat(secret_dest_buffer, argv[3]);
            encInfo.secret_fname = secret_dest_buffer;
            char *str = strstr(argv[3], ".");
            encInfo.ext_size = (str != NULL) ? strlen(str) : 0;
            encInfo.ext = str;
        }
        else
        {
            fprintf(stderr, "Error: Secret file path too long.\n");
            return 1;
        }

        if (argc > 4) // Check if argv[4] exists
        {
            if (strlen(base__encode_op) + strlen(argv[4]) < MAX_PATH_LENGTH)
            {
                strcpy(encode_dest_buffer, base__encode_op);
                strcat(encode_dest_buffer, argv[4]);
                encInfo.stego_image_fname = encode_dest_buffer;
            }
            else
            {
                fprintf(stderr, "Error: Stego image path too long.\n");
                return 1;
            }
        }
        else
        {
            strcpy(default_stego_path, default_encode_stego);
            encInfo.stego_image_fname = default_stego_path;
        }

        if (open_files(&encInfo) == e_failure)
        {
            fprintf(stdout, "ERROR: %s function failed\n", "open_files");
            return 1;
        }

        if (check_capacity(&encInfo) == e_failure)
        {
            fprintf(stdout, "ERROR: function failed - %s \tthe size of the message exceeded the capacity of the image\n", "check_capacity");
            fclose(encInfo.fptr_src_image);
            fclose(encInfo.fptr_secret);
            fclose(encInfo.fptr_stego_image);
            return 1;
        }


        if (do_encoding(&encInfo) == e_failure)
        {
            fprintf(stdout, "\nThe encoding wasnt successful!");
        }
    }
    else if (strcmp(argv[1], "-d") == 0)
    {

        if (argv[2] == NULL)
        {
            fprintf(stdout, "ERROR : Too few arguments, check --info for more details\n");
            return e_failure;
        }

        char decode_img_src_buffer[MAX_PATH_LENGTH];
        char secret_dest_buffer[MAX_PATH_LENGTH];
        char default_secret_path[MAX_PATH_LENGTH];

        if (strlen(base__decode_ip) + strlen(argv[2]) < MAX_PATH_LENGTH)
        {
            strcpy(decode_img_src_buffer, base__decode_ip);
            strcat(decode_img_src_buffer, argv[2]);
            encInfo.stego_image_fname = decode_img_src_buffer;
        }
        else
        {
            fprintf(stderr, "Error: Source image path too long.\n");
            return 1;
        }

        if (argc > 3) // Check if argv[4] exists
        {
            if (strlen(base__decode_op) + strlen(argv[3]) < MAX_PATH_LENGTH)
            {
                strcpy(secret_dest_buffer, base__decode_op);
                strcat(secret_dest_buffer, argv[3]);
                encInfo.dest_file = secret_dest_buffer;
            }
            else
            {
                fprintf(stderr, "Error: Stego image path too long.\n");
                return 1;
            }
        }
        else
        {
            strcpy(default_secret_path, default_decode_secret);
            encInfo.dest_file = default_secret_path;
        }

        if (open_decode_files(&encInfo) == e_failure)
        {
            fprintf(stdout, "ERROR: %s function failed\n", "open_files");
            return 1;
        }
        if (do_decoding(&encInfo) == e_failure)
        {
            fprintf(stdout, "\nThe decoding was not successful!\n");
        }
    }
    else
    {
        fprintf(stdout, "invalid arguments passed check --info for usage options.\n");
        return e_failure;
    }

    fflush(stdout);
    return 0;
}
