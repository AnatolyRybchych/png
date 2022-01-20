#ifndef __PNG_H_
#define __PNG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct __PNG_CHUNK_T png_chunk_t;

//src -> png data with header in the start
//returns false if src has no png header
//foreach png chunk calls on_png_chunk with user params
bool png_foreach_chunk(void *src, uint32_t src_size, void (*on_png_chunk)(const png_chunk_t *chunk, void *user_params), void *user_params);

struct __PNG_CHUNK_T
{
    uint32_t lengh;       // bytes count of data, can be 0
    uint32_t chunk_type;  // PNG_CHUNK_TYPES instance { IHDR, IDAT, IEND, PLET,..} 
    uint8_t *data;        // can be NULL
    uint32_t CRC;         // (Cyclic Redundancy Check) calculated on the preceding bytes in the chunk
};

enum PNG_CHUNK_TYPES
{
    IHDR,
    IEND,
    IDAT,
    PLTE,
    CHUNK_TYPES_CNT,        
};

#endif