#ifndef __PNG_H_
#define __PNG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct __PNG_CHUNK_T png_chunk_t;
typedef struct __IHDR_CONTENT IHDR_content;

//src -> png data with header in the start
//returns false if src has no png header
//foreach png chunk calls on_png_chunk with user params
bool png_foreach_chunk(void *src, uint32_t src_size, void (*on_png_chunk)(const png_chunk_t *chunk, void *user_params), void *user_params);

//returns false ifchunk type is not IHDR 
bool png_translate_IHDR(const png_chunk_t *chunk, IHDR_content *result);

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

struct __IHDR_CONTENT
{
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression_method;
    uint8_t filter_method;
};

#endif