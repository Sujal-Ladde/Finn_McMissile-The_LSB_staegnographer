#include "decode.h"
#include "common.h" // For str_to_int, etc.
#include <cstdio>
#include <cstring>
#include <cstdlib> // For malloc/free, though new/delete is more C++ idiomatic for arrays
#include <vector>  // Alternatively, std::vector can be used for safer dynamic arrays

Status open_decode_files(EncodeInfo *encInfo)
{
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "rb"); // rb for reading
    if (encInfo->fptr_stego_image == NULL)
    {
        return e_failure;
    }
    return e_success;
}

uint8_t decode_magic_size(EncodeInfo *encInfo)
{
    uint8_t magic_s = 0;
    if (decode_data_from_image(1, encInfo->fptr_stego_image, (char*)&magic_s) == e_failure)
    {
        return 0; 
    }
    return magic_s;
}

Status extract_magic(EncodeInfo *encInfo, const char *magic_string_arg)
{
    uint8_t size = decode_magic_size(encInfo);
    if (size == 0) { 
        return e_failure;
    }
    if (size >= sizeof(encInfo->MAGIC_STRING)) { 
        return e_failure;
    }

    // char extracted_magic[size + 1]; // VLA not supported by MSVC
    char *extracted_magic = new (std::nothrow) char[size + 1]; // Dynamic allocation
    if (!extracted_magic) {
        return e_failure; // Allocation failed
    }

    if (decode_data_from_image(size, encInfo->fptr_stego_image, extracted_magic) == e_failure)
    {
        delete[] extracted_magic; // Clean up
        return e_failure;
    }
    extracted_magic[size] = '\0';

    Status result = e_success;
    if (strcmp(extracted_magic, magic_string_arg) != 0)
    {
        result = e_failure;
    }
    
    delete[] extracted_magic; // Clean up
    return result;
}

uint8_t decode_file_extn_size(EncodeInfo *encInfo)
{
    uint8_t extn_s = 0; 
    if (decode_data_from_image(1, encInfo->fptr_stego_image, (char*)&extn_s) == e_failure)
    {
        return 0; 
    }
    return extn_s;
}

Status decode_file_extension(EncodeInfo *encInfo)
{
    uint8_t extn_size = decode_file_extn_size(encInfo);
    if (extn_size == 0) { 
        return e_failure; 
    }
    
    if (extn_size > 20) {  // Max reasonable extension size
        return e_failure;
    }

    // char file_ext[extn_size + 1]; // VLA not supported by MSVC
    char *file_ext = new (std::nothrow) char[extn_size + 1]; // Dynamic allocation
    if (!file_ext) {
        return e_failure; // Allocation failed
    }

    if (decode_data_from_image(extn_size, encInfo->fptr_stego_image, file_ext) == e_failure)
    {
        delete[] file_ext; // Clean up
        return e_failure;
    }
    file_ext[extn_size] = '\0';

    if (encInfo->ext) { // Free previous extension if any (though unlikely in this flow)
        free(encInfo->ext);
    }
    // For MSVC, use _strdup if strdup gives a warning. 
    // If _CRT_SECURE_NO_WARNINGS is defined, strdup should be fine.
    // Or, more portably, implement it or use std::string.
    // Since _CRT_SECURE_NO_WARNINGS will be defined, strdup should be okay.
    // If not, you can use _strdup(file_ext) for MSVC.
#ifdef _MSC_VER
    encInfo->ext = _strdup(file_ext); // Use _strdup for MSVC
#else
    encInfo->ext = strdup(file_ext);
#endif    
    delete[] file_ext; // Clean up buffer

    if (!encInfo->ext) { // strdup failed (out of memory)
         return e_failure;
    }
    return e_success;
}

Status open_dest_file(EncodeInfo *encInfo, const char *name)
{
    encInfo->fptr_dest_file = fopen(name, "wb");
    if (encInfo->fptr_dest_file == NULL)
    {
        return e_failure;
    }
    return e_success;
}


int secret_data_size(EncodeInfo *encInfo) 
{
    char size_bytes[4];
    if (decode_data_from_image(4, encInfo->fptr_stego_image, size_bytes) == e_failure)
    {
        return -1; 
    }
    uint s_size = str_to_int(size_bytes);
    return (int)s_size;
}

Status decode_data_from_image(int size, FILE *fptr_stego_image, char *dest)
{
    if (fptr_stego_image == NULL || dest == NULL || size <= 0)
    {
        return e_failure;
    }

    char image_byte_buffer;
    for (int j = 0; j < size; j++) 
    {
        char reconstructed_byte = 0;
        for (int i = 0; i < 8; i++) 
        {
            if (fread(&image_byte_buffer, 1, 1, fptr_stego_image) != 1)
            {
                return e_failure;
            }
            reconstructed_byte |= ((image_byte_buffer & 1) << i);
        }
        dest[j] = reconstructed_byte;
    }
    return e_success;
}

Status decode_secret_data(EncodeInfo *encInfo)
{
    int data_size = secret_data_size(encInfo);
    if (data_size < 0) { 
        return e_failure;
    }
    if (data_size == 0) { 
        return e_success; 
    }

    const int MAX_DECODE_SIZE = 100 * 1024 * 1024; 
    if (data_size > MAX_DECODE_SIZE) {
        return e_failure;
    }

    // char *secret_data_buf = (char*)malloc(data_size); // C-style
    char *secret_data_buf = new (std::nothrow) char[data_size]; // C++ style, nothrow version
    if (!secret_data_buf) {
        return e_failure;
    }

    if (decode_data_from_image(data_size, encInfo->fptr_stego_image, secret_data_buf) == e_failure)
    {
        delete[] secret_data_buf; // free(secret_data_buf) if malloc was used
        return e_failure;
    }

    if (fwrite(secret_data_buf, 1, data_size, encInfo->fptr_dest_file) != (size_t)data_size)
    {
        delete[] secret_data_buf; // free(secret_data_buf)
        return e_failure;
    }
    delete[] secret_data_buf; // free(secret_data_buf)
    return e_success;
}


Status do_decoding(EncodeInfo *encInfo, const char *magic_string_arg)
{
    // This function is mostly called by the pybind wrapper (bindings.cpp)
    // In bindings.cpp, the sequence is:
    // open_decode_files (done by wrapper if calling directly)
    // fseek
    // extract_magic
    // decode_file_extension
    // (wrapper constructs full output path and calls open_dest_file)
    // decode_secret_data

    // Assuming fptr_stego_image is already open and seeked by the wrapper.
    // Assuming fptr_dest_file will be opened by the wrapper after ext is known.

    // The logic below is somewhat redundant if bindings.cpp calls these stages.
    // However, if do_decoding is intended as a monolithic call, it needs adjustment.
    // Given the current bindings.cpp structure, much of this function's orchestration
    // is handled there. Let's assume the critical parts are `extract_magic`,
    // `decode_file_extension`, and `decode_secret_data`, which are called by bindings.cpp.

    // For direct call to do_decoding (not current pybind flow):
    /*
    rewind(encInfo->fptr_stego_image); 
    if (fseek(encInfo->fptr_stego_image, 54, SEEK_SET) != 0)
    {
        return e_failure;
    }
    */

    // These are the core steps called by bindings.cpp after setup:
    if (extract_magic(encInfo, magic_string_arg) == e_failure)
    {
        return e_failure;
    }

    if (decode_file_extension(encInfo) == e_failure) // This will set encInfo.ext
    {
        // encInfo.ext might be NULL if allocation failed inside, or strdup failed
        return e_failure;
    }

    // At this point, bindings.cpp would use encInfo.ext to form the full
    // output path and call open_dest_file(&encInfo, full_output_path).
    // Then it calls decode_secret_data.

    if (encInfo->fptr_dest_file == NULL) { 
        // This indicates the wrapper didn't open the destination file before calling decode_secret_data,
        // which would be an issue if decode_secret_data is called *through* a full do_decoding.
        // However, bindings.cpp calls decode_secret_data directly *after* opening the file.
        // So this check is more for a scenario where do_decoding is called as one unit.
        if(encInfo->ext) { free(encInfo->ext); encInfo->ext = NULL; }
        return e_failure; 
    }


    if (decode_secret_data(encInfo) == e_failure)
    {
        if(encInfo->ext) { free(encInfo->ext); encInfo->ext = NULL; }
        return e_failure;
    }
    
    if(encInfo->ext) { // Clean up allocated extension string if it exists
        free(encInfo->ext); 
        encInfo->ext = NULL;
    }

    return e_success;
}