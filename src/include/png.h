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

    bKGD,//gives the default background color. It is intended for use when there is no better choice available,
        //such as in standalone image viewers (but not web browsers; see below for more details).

    cHRM,//gives the chromaticity coordinates of the display primaries and white point.

    dSIG,//is for storing digital signatures.

    eXIf,//stores Exif metadata

    gAMA,//specifies gamma. The gAMA chunk contains only 4 bytes, 
        //and its value represents the gamma value multiplied by 100,000; 
        //for example, the gamma value 1/3.4 calculates to 29411.7647059 ((1/3.4)*(100,000)) 
        //and is converted to an integer (29412) for storage.

    hIST,//can store the histogram, or total amount of each color in the image

    iCCP,//is an ICC color profile

    iTXt,//contains a keyword and UTF-8 text,
        //with encodings for possible compression and translations marked with language tag. 
        //The Extensible Metadata Platform (XMP) uses this chunk with a keyword 'XML:com.adobe.xmp'

    pHYs,//holds the intended pixel size (or pixel aspect ratio); 
        //the pHYs contains "Pixels per unit, X axis" (4 bytes), "Pixels per unit, Y axis" (4 bytes), 
        //and "Unit specifier" (1 byte) for a total of 9 bytes.

    sBIT,//(significant bits) indicates the color-accuracy of the source data;
        //this chunk contains a total of between 1 and 13 bytes.

    sPLT,//suggests a palette to use if the full range of colors is unavailable.

    sRGB,//indicates that the standard sRGB color space is used; 
        //the sRGB chunk contains only 1 byte, which is used for 
        //"rendering intent" (4 values—0, 1, 2, and 3—are defined for rendering intent).

    sTER,//stereo-image indicator chunk for stereoscopic images.

    tEXt,//can store text that can be represented in ISO/IEC 8859-1, 
        //with one key-value pair for each chunk. The "key" must be between 1 and 79 characters long. 
        //Separator is a null character. The "value" can be any length, 
        //including zero up to the maximum permissible chunk size minus the length of the keyword and separator. 
        //Neither "key" nor "value" can contain null character. Leading or trailing spaces are also disallowed.

    tIME,//stores the time that the image was last changed.

    tRNS,//contains transparency information. For indexed images, 
        //it stores alpha channel values for one or more palette entries. 
        //For truecolor and grayscale images, 
        //it stores a single pixel value that is to be regarded as fully transparent.

    zTXt,//contains compressed text (and a compression method marker) with the same limits as tEXt.

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