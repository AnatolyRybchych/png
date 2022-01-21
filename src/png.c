#include "include/png.h"

//private

#define PNG_FILE_HDR_SIZE 8
#define PNG_CHUNK_NO_DATA_SIZE 12
#define CHUNK_TYPE_SIZE 4

#define IHDR_WIDTH_OFFSET 4
#define IHDR_HEIGHT_OFFSET 0
#define IHDR_SIZE_END_OFFSET 8

typedef union
{
    uint8_t bytes[CHUNK_TYPE_SIZE];
    uint32_t val;
} chunk_type, chunk_data_len, chunk_CRC, IHDR_width, IHDR_height;

//return true if first 8 bytes of data == png_file_header
static bool has_png_file_header(const uint8_t *data);

//use free(png_chunk.data) or free_chunk(&png_chunk) after using chunk
static bool read_chunk(uint8_t *src, png_chunk_t *dst, uint32_t bytes_to_buffer_end);
static void free_chunk(const png_chunk_t *chunk);

//returns NULL if cannot find chunk before the end
//returns next pointer to chunk, that has png header
static void *goto_next_chunk(void *data_ptr, uint32_t bytes_to_buffer_end);

static uint8_t png_file_header[PNG_FILE_HDR_SIZE] = {137, 80, 78, 71, 13, 10, 26, 10};
static chunk_type ChunkTypes[CHUNK_TYPES_CNT] = {
    [IHDR].bytes = {'I', 'H', 'D', 'R'},
    [IDAT].bytes = {'I', 'D', 'A', 'T'},
    [IEND].bytes = {'I', 'E', 'N', 'D'},
    [PLTE].bytes = {'P', 'L', 'T', 'E'},
};



static bool has_png_file_header(const uint8_t *data)
{
    bool res = true;

    for(int i = 0; i< PNG_FILE_HDR_SIZE; i++)
        if(data[i] != png_file_header[i]) res = false;
    
    return res;
}

//returns next address of png chunk after data_ptr
//returns NULL if address is not found in [data_ptr; data_ptr + bytes_to_buffer_end - PNG_CHUNK_NO_DATA_SIZE]  
static void *goto_next_chunk(void *data_ptr, uint32_t bytes_to_buffer_end)
{
    uint8_t *ptr = (uint8_t*)data_ptr;
    uint8_t *end = (uint8_t*)(ptr + bytes_to_buffer_end);

    while (ptr + PNG_CHUNK_NO_DATA_SIZE  <= end)
    {
        chunk_type type = {
            .bytes = {
                [0] = *(ptr + 4),
                [1] = *(ptr + 5),
                [2] = *(ptr + 6),
                [3] = *(ptr + 7),
            },
        };

        for(int i = 0; i< CHUNK_TYPES_CNT; i++)
        {
            if(type.val == ChunkTypes[i].val) 
            {
                return ptr;
            }
        }
        ptr++;
    }

    return NULL;
}

static bool read_chunk(uint8_t *src, png_chunk_t *dst, uint32_t bytes_to_buffer_end)
{
    if(!src) return false;
    if(bytes_to_buffer_end < PNG_CHUNK_NO_DATA_SIZE) return false;
    
    png_chunk_t res;
    uint32_t src_off = 0;

    chunk_data_len len;
    chunk_type type;
    chunk_CRC CRC;

    //chunk data lengh
    for(int i = 0; i < sizeof(len); i++, src_off++)
        len.bytes[sizeof(len) - i - 1] = *(src + src_off);
    len.val = len.val;
    if(len.val > bytes_to_buffer_end - PNG_CHUNK_NO_DATA_SIZE) len.val = 0;
    res.lengh = len.val;

    //chunk type
    for(int i = 0; i < sizeof(type); i++, src_off++)
        type.bytes[i] = *(src + src_off);
    for(int i = 0; i< CHUNK_TYPES_CNT; i++)
        if(type.val == ChunkTypes[i].val)
            res.chunk_type = i;

    //chunk data
    res.data = malloc(len.val);
    if(res.data) memcpy(res.data, src + src_off, len.val);
    src_off += len.val;

    //chunk SRC
    for(int i = 0; i < sizeof(CRC); i++, src_off++)
        CRC.bytes[i] = *(src + src_off);
    res.CRC = CRC.val;
    
    
    *dst = res;
    return true;
}

static void free_chunk(const png_chunk_t *chunk)
{
    if (chunk->data) free(chunk->data);
}


//publics

bool png_foreach_chunk(void *src, uint32_t src_size, void (*on_png_chunk)(const png_chunk_t *chunk, void *user_params), void *user_params)
{
    if(!has_png_file_header(src)) return false;

    png_chunk_t chunk;
    uint32_t to_end = src_size;
    uint8_t *data_ptr = src;

    while ((data_ptr = goto_next_chunk(data_ptr, to_end)) != NULL)
    {
        if(read_chunk(data_ptr, &chunk, to_end))
        {
            on_png_chunk(&chunk, user_params);
            free_chunk(&chunk);
        }
        data_ptr += chunk.lengh + PNG_CHUNK_NO_DATA_SIZE;
        to_end = src_size - (data_ptr - (uint8_t*)src);
    } 

    return true;
}

bool png_translate_IHDR(const png_chunk_t *chunk, IHDR_content *result)
{
    if(chunk->chunk_type != IHDR) return false;
    
    IHDR_width width;
    IHDR_height height;

    width.bytes[3] = chunk->data[IHDR_WIDTH_OFFSET + 0];
    width.bytes[2] = chunk->data[IHDR_WIDTH_OFFSET + 1];
    width.bytes[1] = chunk->data[IHDR_WIDTH_OFFSET + 2];
    width.bytes[0] = chunk->data[IHDR_WIDTH_OFFSET + 3];
    result->width = width.val;

    height.bytes[3] = chunk->data[IHDR_HEIGHT_OFFSET + 0];
    height.bytes[2] = chunk->data[IHDR_HEIGHT_OFFSET + 1];
    height.bytes[1] = chunk->data[IHDR_HEIGHT_OFFSET + 2];
    height.bytes[0] = chunk->data[IHDR_HEIGHT_OFFSET + 3];
    result->height = height.val;

    result->bit_depth = chunk->data[IHDR_SIZE_END_OFFSET + 0];
    result->color_type = chunk->data[IHDR_SIZE_END_OFFSET + 1];
    result->compression_method = chunk->data[IHDR_SIZE_END_OFFSET + 2];
    result->filter_method = chunk->data[IHDR_SIZE_END_OFFSET + 3];

    return true;
}