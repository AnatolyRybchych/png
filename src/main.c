#include<stdio.h>

#include "include/png.h"

//allocs buffer and fills by file data
//size -> buffer size
uint8_t *get_file_data(const char *file_name, int32_t *size);

int main(int argc, char* argv[])
{
    int32_t file_size;
    uint8_t *file_data = get_file_data("cat.png", &file_size);

    if(has_png_file_header(file_data))
    {
        uint8_t *data_ptr = file_data;
        png_chunk_t chunk;

        uint32_t to_end = file_size;
        uint8_t prev = data_ptr;
        while ((data_ptr = goto_next_chunk(data_ptr, to_end)) != NULL)
        {

            if(prev == data_ptr)
                data_ptr ++;
            prev = data_ptr;
            
            if(read_chunk(data_ptr, &chunk, to_end))
            {
                printf("%.*s\n", 4, chunk.chunk_type.bytes);
                printf("size: %i\n", chunk.lengh.val);
                data_ptr += chunk.lengh.val + 12;
            }
            to_end = file_size - (data_ptr - file_data);
        } 
    }
    else
    {
        fprintf(stderr, "file has no png format\n");
        return -1;
    }
    

    return 0;
}

uint8_t *get_file_data(const char *file_name, int32_t *size)
{
    int32_t file_size;
    uint8_t *data;
    FILE* file;

    file = fopen(file_name, "rb");
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    data = (uint8_t*)malloc(file_size);
    fread(data, file_size, 1, file);

    fclose(file);
    *size = file_size;
    return data;
}