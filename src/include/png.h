#ifndef __PNG_H_
#define __PNG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct __PNG_CHUNK_T png_chunk_t;
typedef struct __IHDR_CONTENT IHDR_content;
typedef void (*png_filter_proc)(void *src, void *prev_src, uint32_t src_size, uint8_t bpp);

//src -> png data with header in the start
//returns false if src has no png header
//foreach png chunk calls on_png_chunk with user params
bool png_foreach_chunk(void *src, uint32_t src_size, void (*on_png_chunk)(const png_chunk_t *chunk, void *user_params), void *user_params);

//returns false ifchunk type is not IHDR 
bool png_translate_IHDR(const png_chunk_t *chunk, IHDR_content *result);

//returns false if not
//returns false if type/depth has broken value
bool png_is_color_type_accepts_depth(uint8_t color_type, uint8_t depth);

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

enum PNG_FILTER_TYPES
{
    FILTER_NONE = 0,
    FILTER_SUB = 1,
    FILTER_UP = 2,
    FILTER_AVERAGE = 3,
    FILTER_PEATH = 4,
    FILTERS_CNT,
};

enum PNG_COLOR_TYPES
{
    COLOR_GRAY_FLAG = 0,
    COLOR_USER_PLTE_FLAG = 1,
    COLOR_RGB_FLAG = 2,
    COLOR_ALPHA_FLAG = 4,

    COLOR_GRAY_SCALE = COLOR_GRAY_FLAG,
    COLOR_RGB_TRIPLE = COLOR_RGB_FLAG,
    COLOR_PLTE = COLOR_USER_PLTE_FLAG | COLOR_RGB_FLAG,
    COLOR_ALPHA_GRAYSCALE = COLOR_GRAY_FLAG | COLOR_ALPHA_FLAG,
    COLOR_ALPHA_RGB_TRIPLE = COLOR_RGB_FLAG | COLOR_ALPHA_FLAG,
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

extern png_filter_proc Png_filters[FILTERS_CNT];
extern png_filter_proc Png_reverse_filters[FILTERS_CNT];

#endif