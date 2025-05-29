// encode.cpp
#include "encode.h"
#include <cstdio>
#include <cstring>
#include <cstdint>

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image) {
    uint width, height;
    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);
    return width * height * 3;
}

Status open_files(EncodeInfo *encInfo) {
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (!encInfo->fptr_src_image) {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (!encInfo->fptr_secret) {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (!encInfo->fptr_stego_image) {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        return e_failure;
    }
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo) {
    fseek(encInfo->fptr_src_image, 0, SEEK_END);
    long available = ftell(encInfo->fptr_src_image) - 54 - 1;
    printf("the size of the image is %ld\n", available);
    int secret_size = get_file_size(encInfo->fptr_secret);
    printf("the size of the secret is %d\n", secret_size);
    if (secret_size > (available / 8)) {
        printf("the secret is too big\n");
        return e_failure;
    }
    rewind(encInfo->fptr_src_image);
    fprintf(stdout, "LOG: Extracter the file extention and created the destion file\n");
    return e_success;
}

uint get_file_size(FILE *fptr) {
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    return size;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image) {
    char header[54];
    if (fread(header, 54, 1, fptr_src_image) != 1) {
        fprintf(stderr, "ERROR: Unable to read the header!\n");
        return e_failure;
    }
    if (fwrite(header, 54, 1, fptr_dest_image) != 1) {
        fprintf(stderr, "ERROR: Unable to write the header!\n");
        return e_failure;
    }
    fprintf(stdout, "LOG: successfully encoded the header\n");
    return e_success;
}

Status encode_magic_string(EncodeInfo *encInfo, const char *magic_string_arg) {
    if (!magic_string_arg) {
        fprintf(stderr, "ERROR: Magic string argument is null!\n");
        return e_failure;
    }
    encInfo->magic_size = static_cast<uint8_t>(strlen(magic_string_arg));
    if (encInfo->magic_size == 0) {
        fprintf(stderr, "ERROR: The size of the magic string is invalid!\n");
        return e_failure;
    }
    strcpy(encInfo->MAGIC_STRING, magic_string_arg);
    if (encode_data_to_image(reinterpret_cast<const char *>(&encInfo->magic_size), 1,
                              encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) {
        fprintf(stderr, "ERROR: Failed to encode the size of the magic string!\n");
        return e_failure;
    }
    if (encode_data_to_image(magic_string_arg, encInfo->magic_size,
                              encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) {
        fprintf(stderr, "ERROR: Failed to encode the magic string!\n");
        return e_failure;
    }
    fprintf(stdout, "LOG: %s successfully encoded the magic string\n", magic_string_arg);
    return e_success;
}

Status encode_secret_file_extn_size(uint8_t ext_size, EncodeInfo *encInfo) {
    char c = static_cast<char>(ext_size);
    if (encode_data_to_image(&c, 1, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) {
        fprintf(stderr, "ERROR: Failed to encode the size of the extension!\n");
        return e_failure;
    }
    fprintf(stdout, "LOG: successfully encoded the size of the extension\n");
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo) {
    if (encode_data_to_image(file_extn, strlen(file_extn),
                              encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) {
        fprintf(stderr, "ERROR: Failed to encode the extension!\n");
        return e_failure;
    }
    fprintf(stdout, "LOG: successfully encoded the extension\n");
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo) {
    uint size_uint = static_cast<uint>(file_size);
    char *size_str = int_to_str(size_uint);
    if (encode_data_to_image(size_str, 4,
                              encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) {
        fprintf(stderr, "ERROR: Failed to encode size of the secret file!\n");
        return e_failure;
    }
    fprintf(stdout, "LOG: successfully encoded the size of the secret file\n");
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo) {
    int size = get_file_size(encInfo->fptr_secret);
    if (size <= 0) {
        fprintf(stderr, "ERROR: Failed to read the size of the secret file!\n");
        return e_failure;
    }
    char *buffer = new char[size];
    fread(buffer, 1, size, encInfo->fptr_secret);
    if (encode_data_to_image(buffer, size,
                              encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) {
        fprintf(stderr, "ERROR: Failed to encode the secret file!\n");
        delete[] buffer;
        return e_failure;
    }
    delete[] buffer;
    fprintf(stdout, "LOG: successfully encoded the secret file\n");
    return e_success;
}

Status encode_data_to_image(const char *data, int size,
                            FILE *fptr_src_image, FILE *fptr_stego_image) {
    if (!data || !fptr_src_image || !fptr_stego_image || size <= 0) {
        fprintf(stderr, "ERROR: Invalid arguments to encode_data_to_image.\n");
        return e_failure;
    }
    for (int j = 0; j < size; ++j) {
        unsigned char data_byte = static_cast<unsigned char>(data[j]);
        char image_byte;
        for (int i = 0; i < 8; ++i) {
            if (fread(&image_byte, 1, 1, fptr_src_image) != 1) {
                fprintf(stderr, "ERROR: Failed to read a byte from source image.\n");
                return e_failure;
            }
            image_byte = (image_byte & ~1) | ((data_byte >> i) & 1);
            fwrite(&image_byte, 1, 1, fptr_stego_image);
        }
    }
    if (ftell(fptr_src_image) != ftell(fptr_stego_image)) {
        fprintf(stderr, "ERROR: File pointer misalignment after encoding.\n");
        return e_failure;
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest) {
    char byte;
    while (fread(&byte, 1, 1, fptr_src) == 1) {
        fwrite(&byte, 1, 1, fptr_dest);
    }
    fprintf(stdout, "LOG: successfully copied the remaining bits\n");
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo, const char *magic_string_arg) {
    rewind(encInfo->fptr_src_image);
    rewind(encInfo->fptr_secret);

    // Extract the file extension from the secret file name
   // Ensure encInfo->ext is NULL before strdup if it might have been previously allocated
    // or ensure proper memory management if this code can be called multiple times.
    // Assuming encInfo->ext is either NULL or will be correctly freed if re-assigned.
    if (encInfo->secret_fname) {
        const char *dot_ptr = strrchr(encInfo->secret_fname, '.'); // Use const char* for pointer from strrchr

        // Check if a dot is found and it's not the first character of the filename
        // (e.g., to handle Unix-style hidden files like ".bashrc" as having no extension
        // for this purpose, or filenames that are just ".")
        if (dot_ptr && dot_ptr != encInfo->secret_fname && dot_ptr[0] != '\0' /* Ensure dot is not the last char if we need an actual extension after it */ ) {
            // If we specifically want to ensure there are characters *after* the dot for it to be a valid extension:
            // if (dot_ptr && dot_ptr != encInfo->secret_fname && dot_ptr[1] != '\0') {
            // For just capturing the dot and whatever follows:
#ifdef _MSC_VER
            encInfo->ext = _strdup(dot_ptr); // Corrected: copy from the dot itself
#else
            encInfo->ext = strdup(dot_ptr);  // Corrected: copy from the dot itself
#endif
            if (!encInfo->ext) {
                // Handle memory allocation failure for encInfo->ext
                // (e.g., return e_failure or set an error flag)
                // For now, if strdup fails, encInfo->ext will be NULL.
            }
        } else {
            // No dot found, or dot is the first character, or dot is the last character.
            // Treat as no extension.
#ifdef _MSC_VER
            encInfo->ext = _strdup("");
#else
            encInfo->ext = strdup("");
#endif
            if (!encInfo->ext) {
                // Handle memory allocation failure
            }
        }
    } else {
        // No secret filename provided.
#ifdef _MSC_VER
        encInfo->ext = _strdup("");
#else
        encInfo->ext = strdup("");
#endif
        if (!encInfo->ext) {
            // Handle memory allocation failure
        }
    }

    // After this block, encInfo->ext should point to ".ext" or "".
    // Then, encInfo->ext_size should be set:
    if (encInfo->ext) {
        size_t ext_len = strlen(encInfo->ext);
        if (ext_len > UINT8_MAX) {
            // Handle error: extension too long for uint8_t
            // Potentially free encInfo->ext and return an error or set encInfo->ext_size to a special value.
            // For now, this shows the calculation. This check should be robust.
             encInfo->ext_size = UINT8_MAX; // Or handle error more gracefully
        } else {
            encInfo->ext_size = static_cast<uint8_t>(ext_len);
        }
    } else {
        // This case should ideally not be reached if strdup("") is used for fallbacks,
        // unless strdup itself failed and returned NULL.
        encInfo->ext_size = 0; 
    }

    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_stego_image);
        fclose(encInfo->fptr_secret);
        fprintf(stderr, "ERROR: copy_bmp_header failed.");
        return e_failure;
    }
    if (encode_magic_string(encInfo, magic_string_arg) == e_failure) {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_stego_image);
        fclose(encInfo->fptr_secret);
        fprintf(stderr, "ERROR: encode_magic_string failed.");
        return e_failure;
    }
    if (encode_secret_file_extn_size(encInfo->ext_size, encInfo) == e_failure ||
        encode_secret_file_extn(encInfo->ext, encInfo) == e_failure ||
        encode_secret_file_size(get_file_size(encInfo->fptr_secret), encInfo) == e_failure ||
        encode_secret_file_data(encInfo) == e_failure ||
        copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_stego_image);
        fclose(encInfo->fptr_secret);
        fprintf(stderr, "ERROR: Encoding process failed.");
        return e_failure;
    }
    fprintf(stdout, "Encoded successfully");
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_stego_image);
    fclose(encInfo->fptr_secret);
    return e_success;
}
