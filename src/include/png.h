#ifndef __PNG_H_
#define __PNG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define PNG_FILE_HDR_SIZE 8
extern uint8_t png_file_header[PNG_FILE_HDR_SIZE];

bool has_png_file_header(const uint8_t *data);



#endif