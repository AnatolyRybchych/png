#include "include/png.h"

uint8_t png_file_header[PNG_FILE_HDR_SIZE] = {137, 80, 78, 71, 13, 10, 26, 10};

chunk_type ChunkTypes[CHUNK_TYPES_CNT] = {
    [IHDR].bytes = {'I', 'H', 'D', 'R'},
    [IDAT].bytes = {'I', 'D', 'A', 'T'},
    [IEND].bytes = {'I', 'E', 'N', 'D'},
    [PLTE].bytes = {'P', 'L', 'T', 'E'},
};

bool has_png_file_header(const uint8_t *data)
{
    bool res = true;

    for(int i = 0; i< PNG_FILE_HDR_SIZE; i++)
        if(data[i] != png_file_header[i]) res = false;
    
    return res;
}

void *goto_next_chunk(void *data_ptr, uint32_t bytes_to_buffer_end)
{
    uint8_t *ptr = (uint8_t*)data_ptr;
    uint8_t *end = (uint8_t*)(ptr + bytes_to_buffer_end);

    while (ptr != end)
    {
        static int it = 0;
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

bool read_chunk(uint8_t *src, png_chunk_t *dst, uint32_t bytes_to_buffer_end)
{
    if(!src) return false;
    if(bytes_to_buffer_end < PNG_CHUNK_NO_DATA_SIZE) return false;
    png_chunk_t res;
    uint32_t src_off = 0;

    //chunk data lengh
    for(int i = 0; i < sizeof(res.lengh); i++, src_off++)
        res.lengh.bytes[sizeof(res.lengh) - i - 1] = *(src + src_off);
    res.lengh.val = res.lengh.val;
    if(res.lengh.val > bytes_to_buffer_end - PNG_CHUNK_NO_DATA_SIZE) res.lengh.val = 0;

    //chunk type
    for(int i = 0; i < sizeof(res.chunk_type); i++, src_off++)
        res.chunk_type.bytes[i] = *(src + src_off);

    //chunk data
    res.data = malloc(res.lengh.val);
    if(res.data) memcpy(res.data, src + src_off, res.lengh.val);
    src_off += res.lengh.val;

    //chunk SRC
    for(int i = 0; i < sizeof(res.CRC); i++, src_off++)
        res.CRC.bytes[i] = *(src + src_off);

    *dst = res;
    return true;
}

void free_chunk(const png_chunk_t *chunk)
{
    if (chunk->data) free(chunk->data);
}