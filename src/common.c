#include "common.h"


void clear_screen_c() {
    for (int i = 0; i < 100; i++) {
        printf("\n");
    }
}


char *convert_size(uint num)
{
    unsigned char *bytes = (uint8_t *)malloc(4 * sizeof(char));
    bytes[3] = (num >> 24) & 0xFF;
    bytes[2] = (num >> 16) & 0xFF;
    bytes[1] = (num >> 8) & 0xFF;
    bytes[0] = num & 0xFF;

    return bytes;
}