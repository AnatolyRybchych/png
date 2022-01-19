#include<stdio.h>

#include "include/png.h"

//allocs buffer and fills by file data
//size -> buffer size
uint8_t *get_file_data(const char *file_name, int32_t *size);

int main(int argc, char* argv[])
{
    int32_t file_size;
    uint8_t *file_data = get_file_data("cat.png", &file_size);

    uint8_t header[10] = {0};
    memcpy(header, file_data, 9);

    printf("%i\n", has_png_file_header(file_data));
    

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
    return data;
}