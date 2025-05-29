#include "common.h"
#include <cstdio>  // For printf if used for logging
#include <cstdlib> // For malloc
#include <cstring> // For string functions

// void clear_screen_c() { // Remove - not suitable for library
//     for (int i = 0; i < 100; i++) {
//         printf("\n");
//     }
// }

char *int_to_str(uint num)
{
    // Allocate with new in C++ if treating as C++, or stick to malloc/free for C-style.
    // Pybind will manage memory for Python-facing objects, this is internal.
    uint8_t *bytes = (uint8_t *)malloc(4 * sizeof(uint8_t)); // Ensure uint8_t for byte manipulation
    if (!bytes) return nullptr; // Allocation check
    bytes[3] = (num >> 24) & 0xFF;
    bytes[2] = (num >> 16) & 0xFF;
    bytes[1] = (num >> 8) & 0xFF;
    bytes[0] = num & 0xFF;

    return (char*)bytes; // Cast to char* as per original signature
}

uint str_to_int(const char* data) // Make data const
{
 const uint8_t* u_data = (const uint8_t*)data; // Cast for proper unsigned arithmetic
 return (uint)(u_data[3] << 24) | (uint)(u_data[2] << 16) | (uint)(u_data[1] << 8) | (uint)u_data[0];
}