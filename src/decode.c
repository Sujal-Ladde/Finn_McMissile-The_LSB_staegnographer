#include "decode.h"

/*
 * Open the necessary files for decoding
 * Input: EncodeInfo structure containing file names
 * Output: None
 * Return Value: e_success if stego image is opened successfully, e_failure otherwise
 */
Status open_decode_files(EncodeInfo *encInfo)
{
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "r"); // Open the stego image in read mode
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }
    return e_success; // Stego image opened successfully
}

/*
 * Decode the size of the magic string from the stego image
 * Input: EncodeInfo structure containing the stego image file pointer
 * Output: Size of the magic string (uint8_t)
 * Return Value: Decoded magic string size, 0 on failure
 */
uint8_t decode_magic_size(EncodeInfo *encInfo)
{
    uint8_t magic_size = 0;
    if (decode_data_from_image(1, encInfo->fptr_stego_image, &magic_size) == e_failure) // Decode 1 byte for the magic string size
    {
        fprintf(stderr, "ERROR : unable to extract size of the magic string!\n");
        return 0;
    }
    fprintf(stdout, "LOG: Decoded size of the magic string\n");
    return magic_size; // Return the decoded magic string size
}

/*
 * Extract and verify the magic string from the stego image
 * Input: EncodeInfo structure containing the stego image file pointer
 * Output: None
 * Return Value: e_success if magic string is extracted and verified, e_failure otherwise
 */
Status extract_magic(EncodeInfo *encInfo)
{
    char magic[50]; // Buffer to store the user-entered magic string
    fprintf(stdout,"enter magic string : ");
    scanf("%s", magic); // Get the magic string from the user

    char str[8];                               // Unused variable
    uint8_t size = decode_magic_size(encInfo); // Get the decoded magic string size
    if (size == 0)
        return e_failure;
 
    char extracted_magic[size + 1]; // Buffer to store the extracted magic string (including null terminator)

    if (decode_data_from_image(size, encInfo->fptr_stego_image, extracted_magic) == e_failure) // Decode the magic string
    {
        fprintf(stderr, "ERROR : unable to extract magic string!\n");
        return e_failure;
    }

    extracted_magic[size] = '\0';            // Null-terminate the extracted magic string
    if (strcmp(extracted_magic, magic) != 0) // Compare extracted and user-entered magic strings
    {
        fprintf(stderr, "ERROR : the entered magic string is incorrect!\n");
        return e_failure;
    }
    fprintf(stdout, "LOG: authenticated the entered magic string\n");
    return e_success; // Magic string verified successfully
}

/*
 * Decode the size of the secret file extension from the stego image
 * Input: EncodeInfo structure containing the stego image file pointer
 * Output: Size of the file extension (uint8_t)
 * Return Value: Decoded file extension size, 0 on failure
 */
uint8_t decode_file_extn_size(EncodeInfo *encInfo)
{
    uint8_t extn_size = 0;
    if (decode_data_from_image(1, encInfo->fptr_stego_image, &extn_size) == e_failure) // Decode 1 byte for the extension size
    {
        fprintf(stderr, "ERROR : unable to extract the size of the file extention!\n");
        return 0;
    }
    fprintf(stdout, "LOG: Extracted the size of the size of the file extention\n");
    return extn_size; // Return the decoded extension size
}

/*
 * Decode the secret file extension from the stego image and open the destination file  
 * Input: EncodeInfo structure containing the stego image and destination file names
 * Output: None
 * Return Value: e_success if extension is decoded and destination file opened, e_failure otherwise
 */
Status decode_file_extension(EncodeInfo *encInfo)
{
    int extn_size = decode_file_extn_size(encInfo); // Get the decoded extension size
    if (extn_size == 0)
    {
        fprintf(stderr, "ERROR : size of the file extention is 0!\n");
        return e_failure;
    }
    char file_ext[extn_size + 1];                                                            // Buffer to store the decoded file extension (including null terminator)
    if (decode_data_from_image(extn_size, encInfo->fptr_stego_image, file_ext) == e_failure) // Decode the file extension
    {
        fprintf(stderr, "ERROR : unable to extract the file extention!\n");
    }
    file_ext[extn_size] = '\0'; // Null-terminate the decoded extension

    uint8_t destfile_size = strlen(encInfo->dest_file);
    char dest_file_name[destfile_size + extn_size + 1]; // Buffer for the complete destination file name

    strcpy(dest_file_name, encInfo->dest_file); // Copy the base destination file name
    strcat(dest_file_name, file_ext);           // Append the extracted file extension

    dest_file_name[destfile_size + extn_size] = '\0'; // Ensure null termination

    if (open_dest_file(encInfo, dest_file_name) == e_failure) // Open the destination file
    {
        fprintf(stderr, "ERROR : unable to open the destination file!\n");
        return e_failure;
    }
    fprintf(stdout, "LOG: Extracted the file extention and created the destion file\n");
    return e_success; // Extension decoded and destination file opened
}

/*
 * Open the destination file for writing the extracted secret data
 * Input: EncodeInfo structure, and the name of the destination file
 * Output: None
 * Return Value: e_success if destination file is opened, e_failure otherwise
 */
Status open_dest_file(EncodeInfo *encInfo, char *name)
{
    encInfo->fptr_dest_file = fopen(name, "wb"); // Open the destination file in write binary mode
    if (encInfo->fptr_dest_file == NULL)
    {
        return e_failure;
    }
    return e_success; // Destination file opened successfully
}

/*
 * Decode the size of the secret data from the stego image
 * Input: EncodeInfo structure containing the stego image file pointer
 * Output: Size of the secret data (int)
 * Return Value: Decoded secret data size, 0 on failure
 */
int secret_data_size(EncodeInfo *encInfo)
{
    char str[8]; // Unused variable
    char x[4];
    if (decode_data_from_image(4, encInfo->fptr_stego_image, x) == e_failure) // Decode 4 byte for the secret data size
    {
        fprintf(stderr, "ERROR : unable to extract the size of the secret data!\n");
        return 0;
    }
    fprintf(stdout, "LOG: extracted the size of the secret data\n");
    uint k=str_to_int(x);
    return k; // Return the decoded secret data size as an integer
}

/*
 * Decode a given number of bytes from the LSBs of the stego image
 * Inputs: Number of bytes to decode, FILE pointer for stego image,
 * pointer to the destination buffer
 * Output: None
 * Return Value: e_success if data is decoded successfully, e_failure otherwise
 */
Status decode_data_from_image(int size, FILE *fptr_stego_image, char *dest)
{
    if (fptr_stego_image == NULL || dest == NULL || size <= 0)
    {
        fprintf(stderr, "ERROR : Invalid arguments to decode_data_from_image.\n");
        return e_failure;
    }

    char str[8];                   // Buffer to read 8 bytes (1 byte of encoded data) from the image
    for (int j = 0; j < size; j++) // Iterate for the number of bytes to decode
    {
        char x = 0;
        if (fread(str, 1, 8, fptr_stego_image) != 8) // Read 8 bytes from the stego image
        {
            if (feof(fptr_stego_image))
            {
                fprintf(stderr, "ERROR : End of file reached prematurely while decoding data byte %d of %d.\n", j + 1, size);
            }
            else
            {
                fprintf(stderr, "ERROR : Failed to read 8 bytes from stego image for data byte %d.\n", j + 1);
            }
            return e_failure;
        }
        for (int i = 0; i < 8; i++) // Extract LSB of each of the 8 bytes
        {
            x |= (str[i] & 1) << i; // Reconstruct the original data byte
        }
        dest[j] = x; // Store the decoded byte in the destination buffer
    }
    return e_success; // Data decoded successfully
}

/*
 * Decode the secret data from the stego image and write it to the destination file
 * Input: EncodeInfo structure containing file pointers
 * Output: None
 * Return Value: e_success if secret data is decoded and written, e_failure otherwise
 */
Status decode_secret_data(EncodeInfo *encInfo)
{
    int data_size = secret_data_size(encInfo); // Get the size of the secret data
    if (data_size == 0)
    {
        fprintf(stderr, "ERROR : size of the file extention is 0!\n");
        return e_failure;
    }
    char x = 0;                                                                                 // Unused variable
    char str[8];                                                                                // Unused variable
    char secret_data[data_size];                                                                // Buffer to store the decoded secret data
    if (decode_data_from_image(data_size, encInfo->fptr_stego_image, secret_data) == e_failure) // Decode the secret data
    {
        fprintf(stderr, "ERROR : unable to extract the secret data!\n");
        return e_failure;
    }
    if (fwrite(secret_data, 1, data_size, encInfo->fptr_dest_file) != data_size) // Write the decoded data to the destination file
    {
        fprintf(stderr, "ERROR : failed to write data to the extracted data to the file !\n");
        return e_failure;
    }
    fprintf(stdout, "LOG: copied the secret data to the file \033[1m%s \033[0msuccessfully!\n", encInfo->dest_file);
    return e_success; // Secret data decoded and written successfully
}

/*
 * Orchestrates the decoding process
 * Input: EncodeInfo structure containing all decoding parameters
 * Output: None
 * Return Value: e_success if decoding is successful, e_failure otherwise
 */
Status do_decoding(EncodeInfo *encInfo)
{
    if (fseek(encInfo->fptr_stego_image, 54, SEEK_SET) != 0) // Seek past the BMP header to the encoded data
    {
        perror("fseek");
        fprintf(stderr, "ERROR: Unable to seek to the start of the encoded data.\n");
        
        return e_failure;
    }

    if (extract_magic(encInfo) == e_failure) // Extract and verify the magic string
    {
        fprintf(stdout,"ERROR: Magic string verification failed. Decoding terminated.\n");
        
        return e_failure;
    }
    clear_screen_c();
    if (decode_file_extension(encInfo) == e_failure) // Decode the file extension and open the destination file
    {
        fprintf(stderr, "ERROR: Failed to decode file extension.\n");
       
        return e_failure;
    }

    if (decode_secret_data(encInfo) == e_failure) // Decode the secret data and write it to the destination file
    {
        fprintf(stderr, "ERROR: Failed to decode secret data.\n");
        
        return e_failure;
    }

    fprintf(stdout, "Decoded successfully\n");
    fclose(encInfo->fptr_stego_image);
    fclose(encInfo->fptr_dest_file);
    return e_success; // Decoding process completed successfully
}