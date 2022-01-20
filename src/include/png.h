#ifndef __PNG_H_
#define __PNG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define PNG_FILE_HDR_SIZE 8
#define PNG_CHUNK_NO_DATA_SIZE 12
#define CHUNK_TYPE_SIZE 4

typedef struct __PNG_CHUNK_T png_chunk_t;
typedef union
{
    uint8_t bytes[CHUNK_TYPE_SIZE];
    uint32_t val;
} chunk_type, chunk_data_len, chunk_CRC;


//return true if first 8 bytes of data == png_file_header
bool has_png_file_header(const uint8_t *data);

//use free(png_chunk.data) or free_chunk(&png_chunk) after using chunk
bool read_chunk(uint8_t *src, png_chunk_t *dst, uint32_t bytes_to_buffer_end);
void free_chunk(const png_chunk_t *chunk);

//returns NULL if cannot find chunk before the end
//returns next pointer to chunk, that has png header
void *goto_next_chunk(void *data_ptr, uint32_t bytes_to_buffer_end);


//types implementatiom

struct __PNG_CHUNK_T
{
    chunk_data_len lengh;     // bytes count of data
    chunk_type chunk_type;    // 4 bytes SCII AZ or decimal 65-90, 87-122
    uint8_t *data;            // can be empty
    chunk_CRC CRC;            // (Cyclic Redundancy Check) calculated on the preceding bytes in the chunk
};

enum PNG_CHUNK_TYPES
{
    IHDR,
    IEND,
    IDAT,
    PLTE,
    CHUNK_TYPES_CNT,        
};

//extern variables

extern chunk_type ChunkTypes[CHUNK_TYPES_CNT];
extern uint8_t png_file_header[PNG_FILE_HDR_SIZE];


#endif