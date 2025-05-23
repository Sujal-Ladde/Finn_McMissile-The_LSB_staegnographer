#include "common.h"


void clear_screen_c() {
    for (int i = 0; i < 100; i++) {
        printf("\n");
    }
}


char *int_to_str(uint num)
{
    unsigned char *bytes = (uint8_t *)malloc(4 * sizeof(char));
    bytes[3] = (num >> 24) & 0xFF;
    bytes[2] = (num >> 16) & 0xFF;
    bytes[1] = (num >> 8) & 0xFF;
    bytes[0] = num & 0xFF;

    return bytes;
}
uint str_to_int(char* data)
{
 return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
}