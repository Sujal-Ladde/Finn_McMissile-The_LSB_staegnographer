#include "encode.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
	uint width, height;
	// Seek to 18th byte
	fseek(fptr_image, 18, SEEK_SET); // Move file pointer to the start of width data

	// Read the width (an int)
	fread(&width, sizeof(int), 1, fptr_image); // Read 4 bytes representing the width
	printf("width = %u\n", width);

	// Read the height (an int)
	fread(&height, sizeof(int), 1, fptr_image); // Read 4 bytes representing the height
	printf("height = %u\n", height);

	// Return image capacity
	return width * height * 3; // Calculate and return the total data capacity of the image
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
	encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb"); // Open source image file in read binary mode
	if (encInfo->fptr_src_image == NULL)
	{
		perror("fopen"); // Print error message to stderr
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
		return e_failure; // Indicate failure to open source image
	}

	encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb"); // Open secret file in read binary mode
	if (encInfo->fptr_secret == NULL)
	{
		perror("fopen"); // Print error message to stderr
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
		fclose(encInfo->fptr_src_image); // Close already opened source image file
		return e_failure;				 // Indicate failure to open secret file
	}

	encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb"); // Open stego image file in write binary mode
	if (encInfo->fptr_stego_image == NULL)
	{
		perror("fopen"); // Print error message to stderr
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
		fclose(encInfo->fptr_src_image); // Close already opened source image file
		fclose(encInfo->fptr_secret);	 // Close already opened secret file
		return e_failure;				 // Indicate failure to open stego image file
	}

	return e_success; // All files opened successfully
}

/*
 * Check if the secret file can be encoded within the source image
 * Input: EncodeInfo structure containing file pointers and names
 * Output: None
 * Return Value: e_success if capacity is sufficient, e_failure otherwise
 */
Status check_capacity(EncodeInfo *encInfo)
{
	fseek(encInfo->fptr_src_image, 0, SEEK_END);	  // Move file pointer to the end of the source image
	long x = ftell(encInfo->fptr_src_image) - 54 - 1; // Calculate available data space (excluding header and potential null terminator)
	printf("the size of the image is %ld\n",x);
	int y=get_file_size(encInfo->fptr_secret);
	printf("the size of the secret is %d\n",y);


	if (y > (x / 8)) // Check if the size of the secret file in bytes exceeds the available bits in the image
	{
		printf("the secret is too big\n");
		return e_failure; // Indicate that the secret file is too large
	}
	rewind(encInfo->fptr_src_image); // Reset the file pointer of the source image to the beginning

	fprintf(stdout, "LOG: Extracter the file extention and created the destion file\n");
	return e_success; // Indicate that the capacity check passed
}

/*
 * Get the size of a given file
 * Input: FILE pointer to the file
 * Output: Size of the file in bytes
 * Description: Seeks to the end of the file to determine its size
 */
uint get_file_size(FILE *fptr)
{
	fseek(fptr, 0, SEEK_END); // Move the file pointer to the end of the file
	uint size = ftell(fptr);  // Get the current position of the file pointer, which is the file size
	fseek(fptr, 0, SEEK_SET); // Reset the file pointer to the beginning of the file
	return (uint)size;		  // Return the calculated file size
}

/*
 * Copy the BMP header from the source image to the stego image
 * Inputs: FILE pointer for the source image, FILE pointer for the stego image
 * Output: None
 * Return Value: e_success if the header is copied successfully, e_failure otherwise
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
	char str[54]; // Buffer to store the 54-byte BMP header

	if (fread(str, 54, 1, fptr_src_image) != 1) // Read 54 bytes from the source image file into the buffer
	{
		fprintf(stderr, "ERROR : Unable to read the header!\n");
		return e_failure; // Indicate failure to read the header
	}
	if (fwrite(str, 54, 1, fptr_dest_image) != 1) // Write 54 bytes from the buffer to the stego image file
	{
		fprintf(stderr, "ERROR : Unable to write the header!\n");
		return e_failure; // Indicate failure to write the header
	}
	fprintf(stdout, "LOG: successfully encoded the header\n" );
	return e_success; // Indicate successful header copy
}

/*
 * Encode the magic string into the stego image
 * Input: EncodeInfo structure containing file pointers and magic string
 * Output: None
 * Return Value: e_success if the magic string is encoded successfully, e_failure otherwise
 */
Status encode_magic_string(EncodeInfo *encInfo)
{
	int j = 0;
	printf("Enter the Magic string:");
	scanf("%s", encInfo->MAGIC_STRING); // Read the magic string from the user input

	encInfo->magic_size = (uint8_t)strlen(encInfo->MAGIC_STRING); // Get the length of the magic string
	if (encInfo->magic_size <= 0)
	{
		fprintf(stderr, "ERROR : The size of the magic string is invalid!\n");
		return e_failure; // Indicate invalid magic string size
	}
	if (encode_data_to_image(&encInfo->magic_size, 1, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) // Encode the size of the magic string (1 byte)
	{
		fprintf(stderr, "ERROR : Failed to encode the size of the magic string!\n");
		return e_failure; // Indicate failure to encode magic string size
	}
	if (encode_data_to_image(encInfo->MAGIC_STRING, strlen(encInfo->MAGIC_STRING), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) // Encode the magic string itself
	{
		fprintf(stderr, "ERROR : Failed to encode the magic string!\n");
		return e_failure; // Indicate failure to encode magic string
	}

	fprintf(stdout, "LOG:%s successfully encode the magic string \n",encInfo->MAGIC_STRING );
	return e_success; // Indicate successful encoding of the magic string
}

/*
 * Encode the size of the secret file extension into the stego image
 * Inputs: Size of the extension, EncodeInfo structure containing file pointers
 * Output: None
 * Return Value: e_success if the size is encoded successfully, e_failure otherwise
 */
Status encode_secret_file_extn_size(uint8_t ext_size, EncodeInfo *encInfo)
{
	char c=ext_size;				
	if (encode_data_to_image(&c, 1, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) // Encode the size of the extension (1 byte)
	{
		fprintf(stderr, "ERROR : Failed to encode the size of the extention!\n");
		return e_failure; // Indicate failure to encode extension size
	}

	fprintf(stdout, "LOG: successfully encoded the size of the extention\n" );
	return e_success; // Indicate successful encoding of the extension size
}

/*
 * Encode the secret file extension into the stego image
 * Inputs: File extension string, EncodeInfo structure containing file pointers
 * Output: None
 * Return Value: e_success if the extension is encoded successfully, e_failure otherwise
 */
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
	if (encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) // Encode the file extension string
	{
		fprintf(stderr, "ERROR : Failed to encode the extention!\n");
		return e_failure; // Indicate failure to encode the extension
	}
	fprintf(stdout, "LOG: successfully encoded the extention\n" );
	return e_success; // Indicate successful encoding of the extension
}

/*
 * Encode the size of the secret file data into the stego image
 * Inputs: Size of the secret file, EncodeInfo structure containing file pointers
 * Output: None
 * Return Value: e_success if the size is encoded successfully, e_failure otherwise
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
	uint c = file_size;		
	char *str=convert_size(c);
	
	if (encode_data_to_image(str, 4, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) // Encode the size of the secret file (1 byte - potential issue for large files)
	{
		fprintf(stderr, "ERROR : Failed to encode size of the secret file!\n");
		return e_failure; // Indicate failure to encode secret file size
	}

	fprintf(stdout, "LOG: successfully encoded the size of the secret file\n");
	return e_success; // Indicate successful encoding of the secret file size
}

/*
 * Encode the actual data of the secret file into the stego image
 * Input: EncodeInfo structure containing file pointers
 * Output: None
 * Return Value: e_success if the data is encoded successfully, e_failure otherwise
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
	int size = get_file_size(encInfo->fptr_secret); // Get the size of the secret file
	if (size <= 0)
	{
		fprintf(stderr, "ERROR : Failed to read the size of the secret file!\n");
		return e_failure; // Indicate failure to read secret file size
	}
	char str[size];																						  // Buffer to hold the content of the secret file
	size_t bytes_read = fread(str, 1, size, encInfo->fptr_secret);										  // Read the content of the secret file into the buffer
	if (encode_data_to_image(str, size, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) // Encode the secret file data into the image
	{
		fprintf(stderr, "ERROR : Failed to encode the secret file!\n");
		return e_failure; // Indicate failure to encode secret file data
	}

	fprintf(stdout, "LOG: successfully encoded the secret file\n");
	return e_success; // Indicate successful encoding of the secret file data
}

/*
 * Encode a given data buffer into the LSBs of the stego image
 * Inputs: Data buffer, size of the data, FILE pointer for source image,
 * FILE pointer for stego image
 * Output: None
 * Return Value: e_success if data is encoded successfully, e_failure otherwise
 */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
	if (fptr_stego_image == NULL || fptr_src_image == NULL || data == NULL || size <= 0)
	{
		fprintf(stderr, "ERROR : Invalid arguments to encode_data_to_image.\n");
		return e_failure; // Indicate invalid input arguments
	}

	for (int j = 0; j < size; j++) // Iterate through each byte of the data to be encoded
	{
		char data_byte = data[j]; // Get the current byte of data
		char image_byte;

		for (int i = 0; i < 8; i++) // Iterate through each bit of the data byte (LSB to MSB)
		{
			if (fread(&image_byte, 1, 1, fptr_src_image) != 1) // Read one byte from the source image
			{
				if (feof(fptr_src_image))
				{
					fprintf(stderr, "ERROR : End of source image reached prematurely while encoding data byte %d of %d.\n", j + 1, size);
				}
				else
				{
					fprintf(stderr, "ERROR : Failed to read a byte from source image for data byte %d.\n", j + 1);
				}
				return e_failure; // Indicate error reading from source image
			}
			// Clear the LSB of the image byte
			image_byte &= ~1;
			// Set the LSB to the current bit of the data byte
			image_byte |= (data_byte >> i) & 1;
			fwrite(&image_byte, 1, 1, fptr_stego_image); // Write the modified image byte to the stego image
		}
	}

	if (ftell(fptr_src_image) != ftell(fptr_stego_image))
	{
		fprintf(stderr, "ERROR : Source and stego image file pointers are misaligned after encoding.\n");
		return e_failure; // Indicate misalignment of file pointers
	}

	return e_success; // Data encoded successfully
}

/*
 * Copy the remaining data from the source image to the stego image
 * Inputs: FILE pointer for the source image, FILE pointer for the stego image
 * Output: None
 * Return Value: e_success if the remaining data is copied successfully, e_failure otherwise
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
	char image_byte;

	while (fread(&image_byte, 1, 1, fptr_src) == 1) // Read one byte at a time from the source image
	{
		fwrite(&image_byte, 1, 1, fptr_dest); // Write the read byte to the stego image
	}

	fprintf(stdout, "LOG: successfully copied the remaining bits\n");
	return e_success; // Indicate successful copying of remaining data
}

/*
 * Orchestrates the encoding process
 * Input: EncodeInfo structure containing all encoding parameters
 * Output: None
 * Return Value: e_success if encoding is successful, e_failure otherwise
 */
Status do_encoding(EncodeInfo *encInfo)
{
	fseek(encInfo->fptr_src_image, 0, SEEK_SET); // Reset the source image file pointer to the beginning
	fseek(encInfo->fptr_secret, 0, SEEK_SET);	 // Reset the secret file pointer to the beginning
	char *txt = encInfo->ext;					 // Hardcoded secret file extension
	int size = get_file_size(encInfo->fptr_secret);

	if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
	{
		fclose(encInfo->fptr_src_image);
		fclose(encInfo->fptr_stego_image);
		fclose(encInfo->fptr_secret);

		fprintf(stderr, "ERROR: copy_bmp_header failed.\n");
		return e_failure; // handle the error as needed
	}

	if (encode_magic_string(encInfo) == e_failure)
	{
		fclose(encInfo->fptr_src_image);
		fclose(encInfo->fptr_stego_image);
		fclose(encInfo->fptr_secret);

		fprintf(stderr, "ERROR: encode_magic_string failed.\n");
		return e_failure; // handle the error as needed
	}
	clear_screen_c();
	if (encode_secret_file_extn_size(strlen(encInfo->ext), encInfo) == e_failure)
	{
		fclose(encInfo->fptr_src_image);
		fclose(encInfo->fptr_stego_image);
		fclose(encInfo->fptr_secret);

		fprintf(stderr, "ERROR: encode_secret_file_extn_size failed.\n");
		return e_failure; // handle the error as needed
	}

	if (encode_secret_file_extn(txt, encInfo) == e_failure)
	{
		fclose(encInfo->fptr_src_image);
		fclose(encInfo->fptr_stego_image);
		fclose(encInfo->fptr_secret);

		fprintf(stderr, "ERROR: encode_secret_file_extn failed.\n");
		return e_failure; // handle the error as needed
	}

	if (encode_secret_file_size(get_file_size(encInfo->fptr_secret), encInfo) == e_failure)
	{
		fclose(encInfo->fptr_src_image);
		fclose(encInfo->fptr_stego_image);
		fclose(encInfo->fptr_secret);

		fprintf(stderr, "ERROR: encode_secret_file_size failed.\n");
		return e_failure; // handle the error as needed
	}

	if (encode_secret_file_data(encInfo) == e_failure)
	{
		fclose(encInfo->fptr_src_image);
		fclose(encInfo->fptr_stego_image);
		fclose(encInfo->fptr_secret);

		fprintf(stderr, "ERROR: encode_secret_file_data failed.\n");
		return e_failure; // handle the error as needed
	}

	if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
	{
		fclose(encInfo->fptr_src_image);
		fclose(encInfo->fptr_stego_image);
		fclose(encInfo->fptr_secret);

		fprintf(stderr, "ERROR: copy_remaining_img_data failed.\n");
		return e_failure; // handle the error as needed
	}

	fprintf(stdout,"Encoded successfully\n");
	fclose(encInfo->fptr_src_image);
	fclose(encInfo->fptr_stego_image);
	fclose(encInfo->fptr_secret);


	return e_success;
}