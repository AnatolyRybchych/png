#include "include/png.h"

uint8_t png_file_header[PNG_FILE_HDR_SIZE] = {137, 80, 78, 71, 13, 10, 26, 10};

bool has_png_file_header(const uint8_t *data)
{
    bool res = true;

    for(int i = 0; i< PNG_FILE_HDR_SIZE; i++)
        if(data[i] != png_file_header[i]) res = false;
    
    return res;
}