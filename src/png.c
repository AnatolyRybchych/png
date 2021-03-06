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
    
    [bKGD].bytes = {'b', 'K', 'G', 'D'},
    [cHRM].bytes = {'c', 'H', 'R', 'M'},
    [dSIG].bytes = {'d', 'S', 'I', 'G'},
    [eXIf].bytes = {'e', 'X', 'I', 'F'},
    [gAMA].bytes = {'g', 'A', 'M', 'A'},
    [hIST].bytes = {'h', 'I', 'S', 'T'},
    [iCCP].bytes = {'i', 'C', 'C', 'P'},
    [iTXt].bytes = {'i', 'T', 'X', 't'},
    [pHYs].bytes = {'p', 'H', 'Y', 's'},
    [sBIT].bytes = {'s', 'B', 'I', 'T'},
    [sPLT].bytes = {'s', 'P', 'L', 'T'},
    [sRGB].bytes = {'s', 'R', 'G', 'B'},
    [sTER].bytes = {'s', 'T', 'E', 'R'},
    [tEXt].bytes = {'t', 'E', 'X', 't'},
    [tIME].bytes = {'t', 'S', 'M', 'E'},
    [tRNS].bytes = {'t', 'R', 'N', 'S'},
    [zTXt].bytes = {'z', 'T', 'X', 't'},
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
    if(len.val > bytes_to_buffer_end - PNG_CHUNK_NO_DATA_SIZE) return false;
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

//filter functions/////////////////////////////////////////////
//filter functions shuold be called for each line of data bytes

static void png_filter_none(void *src, void *prev_src, uint32_t src_size, uint8_t bpp)
{
    return;
}

static void png_filter_sub(void *src, void *prev_src, uint32_t src_size, uint8_t bpp)
{
    uint8_t *src_end = (uint8_t*)src + src_size;

    uint8_t *dst = (uint8_t*)malloc(src_size);

    //out of range src
    for(uint8_t *ptr = src, *dst_ptr = dst; dst_ptr < dst + bpp; ++dst_ptr, ++ptr)
        *dst_ptr = *ptr - 0;

    for(uint8_t *ptr = (uint8_t*)src + bpp, *dst_ptr = dst + bpp; ptr != src_end; ++ptr, ++dst_ptr)
    {
        *dst_ptr = *ptr - *(ptr - bpp); 
    }

    memcpy(src, dst, src_size);
    free(dst);
}

static void png_filter_up(void *src, void *prev_src, uint32_t src_size, uint8_t bpp)
{
    if(prev_src == NULL) return;

    uint8_t *src_end = (uint8_t*)src + src_size;

    for(uint8_t *ptr = src, *prev_ptr = prev_src; ptr != src_end; ++prev_ptr, ++ptr)
        *ptr = *ptr - *prev_ptr;
}

static void png_filter_average(void *src, void *prev_src, uint32_t src_size, uint8_t bpp)
{
    uint8_t *src_end = (uint8_t*)src + src_size;

    uint8_t *sub = (uint8_t*)malloc(src_size);
    uint8_t *prev = (uint8_t*)malloc(src_size);

    memcpy(sub, src, src_size);
    memcpy(prev, src, src_size);

    png_filter_sub(sub, prev_src, src_size, bpp);
    png_filter_up(prev, prev_src, src_size, bpp);

    for(uint8_t *ptr = src, *sub_ptr = sub, *prev_ptr = prev;
        ptr != src_end;
        ++ptr, ++sub_ptr, ++prev_ptr)
        {
            *ptr = *ptr - (uint8_t)floor(((double)*sub + (double)*prev) / 2);
        }

    free(sub);
    free(prev);
}

//https://www.w3.org/TR/PNG-Filters.html 6.6
static void png_filter_peath(void *src, void *prev_src, uint32_t src_size, uint8_t bpp)
{
    uint8_t *src_end = (uint8_t*)src + src_size;

    uint8_t *sub = (uint8_t*)malloc(src_size);      //a arr
    uint8_t *prev = (uint8_t*)malloc(src_size);     //b arr
    uint8_t *prev_sub = (uint8_t*)malloc(src_size); //c arr
    uint8_t *prev_sub_prev = NULL;

    memcpy(sub, src, src_size);
    memcpy(prev, src, src_size);
    memcpy(prev_sub, src, src_size);
    if(prev_src == NULL)
    {
        prev_sub_prev = (uint8_t*)malloc(src_size);
        memcpy(prev_sub_prev + bpp, prev_src, src_size - bpp);
        memset(prev_sub_prev, 0, bpp);
    }

    png_filter_sub(sub, prev_src, src_size, bpp);
    png_filter_up(prev, prev_src, src_size, bpp);
    png_filter_up(prev_sub, prev_sub_prev, src_size, bpp);

    for(uint8_t *ptr = src, *a_ptr = sub, *b_ptr = prev, *c_ptr = prev_sub;
        ptr != src_end;
        ++ptr, ++a_ptr, ++b_ptr, ++c_ptr)
        {
            //PaethPredictor
            int32_t p = *a_ptr + *b_ptr - *c_ptr;

            int32_t pa = abs(p - (int32_t)*a_ptr);
            int32_t pb = abs(p - (int32_t)*b_ptr);
            int32_t pc = abs(p - (int32_t)*c_ptr);

            if(pa <= pb && pa <= pc) *ptr = *ptr -  (uint8_t)*a_ptr;
            else if(pb <= pc) *ptr = *ptr - (uint8_t)*b_ptr;
            else *ptr = *ptr - (uint8_t)*c_ptr;
        }
    
    free(sub);
    free(prev);
    free(prev_sub);
    if(prev_src == NULL) free(prev_sub_prev);
}

static void png_reverse_filter_sub(void *src, void *prev_src, uint32_t src_size, uint8_t bpp)
{
    uint8_t *src_end = (uint8_t*)src + src_size;

    uint8_t *dst = (uint8_t*)malloc(src_size);

    //out of range src
    for(uint8_t *ptr = src, *dst_ptr = dst; dst_ptr < dst + bpp; ++dst_ptr, ++ptr)
        *dst_ptr = *ptr - 0;

    for(uint8_t *ptr = (uint8_t*)src + bpp, *dst_ptr = dst + bpp; ptr != src_end; ++ptr, ++dst_ptr)
    {
        *dst_ptr = *ptr + *(ptr - bpp); 
    }

    memcpy(src, dst, src_size);
    free(dst);
}

static void png_reverse_filter_up(void *src, void *prev_src, uint32_t src_size, uint8_t bpp)
{
    if(prev_src == NULL) return;

    uint8_t *src_end = (uint8_t*)src + src_size;

    for(uint8_t *ptr = src, *prev_ptr = prev_src; ptr != src_end; ++prev_ptr, ++ptr)
        *ptr = *ptr + *prev_ptr;
}

static void png_reverse_filter_average(void *src, void *prev_src, uint32_t src_size, uint8_t bpp)
{
    uint8_t *src_end = (uint8_t*)src + src_size;

    uint8_t *sub = (uint8_t*)malloc(src_size);
    uint8_t *prev = (uint8_t*)malloc(src_size);

    memcpy(sub, src, src_size);
    memcpy(prev, src, src_size);

    png_filter_sub(sub, prev_src, src_size, bpp);
    png_filter_up(prev, prev_src, src_size, bpp);

    for(uint8_t *ptr = src, *sub_ptr = sub, *prev_ptr = prev;
        ptr != src_end;
        ++ptr, ++sub_ptr, ++prev_ptr)
        {
            *ptr = *ptr + (uint8_t)floor(((double)*sub + (double)*prev) / 2);
        }

    free(sub);
    free(prev);
}

//https://www.w3.org/TR/PNG-Filters.html 6.6
static void png_reverse_filter_peath(void *src, void *prev_src, uint32_t src_size, uint8_t bpp)
{
    uint8_t *src_end = (uint8_t*)src + src_size;

    uint8_t *sub = (uint8_t*)malloc(src_size);      //a arr
    uint8_t *prev = (uint8_t*)malloc(src_size);     //b arr
    uint8_t *prev_sub = (uint8_t*)malloc(src_size); //c arr
    uint8_t *prev_sub_prev = NULL;

    memcpy(sub, src, src_size);
    memcpy(prev, src, src_size);
    memcpy(prev_sub, src, src_size);
    if(prev_src == NULL)
    {
        prev_sub_prev = (uint8_t*)malloc(src_size);
        memcpy(prev_sub_prev + bpp, prev_src, src_size - bpp);
        memset(prev_sub_prev, 0, bpp);
    }

    png_filter_sub(sub, prev_src, src_size, bpp);
    png_filter_up(prev, prev_src, src_size, bpp);
    png_filter_up(prev_sub, prev_sub_prev, src_size, bpp);

    for(uint8_t *ptr = src, *a_ptr = sub, *b_ptr = prev, *c_ptr = prev_sub;
        ptr != src_end;
        ++ptr, ++a_ptr, ++b_ptr, ++c_ptr)
        {
            //PaethPredictor
            int32_t p = *a_ptr + *b_ptr - *c_ptr;

            int32_t pa = abs(p - (int32_t)*a_ptr);
            int32_t pb = abs(p - (int32_t)*b_ptr);
            int32_t pc = abs(p - (int32_t)*c_ptr);

            if(pa <= pb && pa <= pc) *ptr = *ptr +  (uint8_t)*a_ptr;
            else if(pb <= pc) *ptr = *ptr + (uint8_t)*b_ptr;
            else *ptr = *ptr + (uint8_t)*c_ptr;
        }
    
    free(sub);
    free(prev);
    free(prev_sub);
    if(prev_src == NULL) free(prev_sub_prev);
}

//filter functions end/////////////////////////////////////////


//publics

png_filter_proc Png_filters[FILTERS_CNT] = {
    [FILTER_NONE] = png_filter_none,
    [FILTER_SUB] = png_filter_sub,
    [FILTER_UP] = png_filter_up,
    [FILTER_AVERAGE] = png_filter_average,
    [FILTER_PEATH] = png_filter_peath,
};

png_filter_proc Png_reverse_filters[FILTERS_CNT] = {
    [FILTER_NONE] = png_filter_none,
    [FILTER_SUB] = png_reverse_filter_sub,
    [FILTER_UP] = png_reverse_filter_up,
    [FILTER_AVERAGE] = png_reverse_filter_average,
    [FILTER_PEATH] = png_reverse_filter_peath,
};

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

bool png_is_color_type_accepts_depth(uint8_t color_type, uint8_t depth)
{
    switch (color_type)
    {
    case COLOR_GRAY_SCALE:
        return depth == 1 || depth == 2 || depth == 4 || depth == 8 || depth == 16;
    case COLOR_RGB_TRIPLE:
        return depth == 8 || depth == 16;
    case COLOR_USER_PLTE_FLAG:
        return depth == 1 || depth == 2 || depth == 4 || depth == 8;
    case COLOR_ALPHA_RGB_TRIPLE:
        return depth == 8 || depth == 16;
    case COLOR_ALPHA_GRAYSCALE:
        return depth == 8 || depth == 16;
    default:
        return false;
    }
}

//context for inflate
struct context {
	const unsigned char *src;
	unsigned int srclen;
	unsigned char *dst;
	unsigned int dstlen;
	unsigned int tag;
	unsigned int bitcnt;
};

static int debit(struct context *ud, unsigned int *result)
{
	unsigned int bit;

	/* check if tag is empty */
	if (!ud->bitcnt--) {
		if (!ud->srclen--) {
			return 0;
		}

		/* load next tag */
		ud->tag = *ud->src++;
		ud->bitcnt = 7;
	}

	/* shift bit out of tag */
	bit = (ud->tag >> 7) & 0x01;
	ud->tag <<= 1;

	*result = bit;

	return 1;
}

static int get_gamma(struct context *ud, unsigned int *result)
{
	unsigned int bit;
	unsigned int v = 1;

	/* input gamma2-encoded bits */
	do {
		if (!debit(ud, &bit)) {
			return 0;
		}

		if (v & 0x80000000) {
			return 0;
		}

		v = (v << 1) + bit;

		if (!debit(ud, &bit)) {
			return 0;
		}
	} while (bit);

	*result = v;

	return 1;
}

bool inflate(const void *src, unsigned int srclen, void *dst, unsigned int dstlen)
{
	struct context ud;
	unsigned int offs, len, R0, LWM, bit;
	int done;
	int i;

	if (!src || !dst) {
		return false;
	}

	ud.src = (const unsigned char *) src;
	ud.srclen = srclen;
	ud.dst = (unsigned char *) dst;
	ud.dstlen = dstlen;
	ud.bitcnt = 0;

	R0 = (unsigned int) -1;
	LWM = 0;
	done = 0;

	/* first byte verbatim */
	if (!ud.srclen-- || !ud.dstlen--) {
		return false;
	}
	*ud.dst++ = *ud.src++;

	/* main decompression loop */
	while (!done) {
		if (!debit(&ud, &bit)) {
			return false;
		}

		if (bit) {
			if (!debit(&ud, &bit)) {
				return false;
			}

			if (bit) {
				if (!debit(&ud, &bit)) {
					return false;
				}

				if (bit) {
					offs = 0;

					for (i = 4; i; i--) {
						if (!debit(&ud, &bit)) {
							return false;
						}
						offs = (offs << 1) + bit;
					}

					if (offs) {
						if (offs > (dstlen - ud.dstlen)) {
							return false;
						}

						if (!ud.dstlen--) {
							return false;
						}

						*ud.dst = *(ud.dst - offs);
						ud.dst++;
					}
					else {
						if (!ud.dstlen--) {
							return false;
						}

						*ud.dst++ = 0x00;
					}

					LWM = 0;
				}
				else {
					if (!ud.srclen--) {
						return false;
					}

					offs = *ud.src++;

					len = 2 + (offs & 0x0001);

					offs >>= 1;

					if (offs) {
						if (offs > (dstlen - ud.dstlen)) {
							return false;
						}

						if (len > ud.dstlen) {
							return false;
						}

						ud.dstlen -= len;

						for (; len; len--) {
							*ud.dst = *(ud.dst - offs);
							ud.dst++;
						}
					}
					else {
						done = 1;
					}

					R0 = offs;
					LWM = 1;
				}
			}
			else {
				if (!get_gamma(&ud, &offs)) {
					return false;
				}

				if ((LWM == 0) && (offs == 2)) {
					offs = R0;

					if (!get_gamma(&ud, &len)) {
						return false;
					}

					if (offs > (dstlen - ud.dstlen)) {
						return false;
					}

					if (len > ud.dstlen) {
						return false;
					}

					ud.dstlen -= len;

					for (; len; len--) {
						*ud.dst = *(ud.dst - offs);
						ud.dst++;
					}
				}
				else {
					if (LWM == 0) {
						offs -= 3;
					}
					else {
						offs -= 2;
					}

					if (offs > 0x00fffffe) {
						return false;
					}

					if (!ud.srclen--) {
						return false;
					}

					offs <<= 8;
					offs += *ud.src++;

					if (!get_gamma(&ud, &len)) {
						return false;
					}

					if (offs >= 32000) {
						len++;
					}
					if (offs >= 1280) {
						len++;
					}
					if (offs < 128) {
						len += 2;
					}

					if (offs > (dstlen - ud.dstlen)) {
						return false;
					}

					if (len > ud.dstlen) {
						return false;
					}

					ud.dstlen -= len;

					for (; len; len--) {
						*ud.dst = *(ud.dst - offs);
						ud.dst++;
					}

					R0 = offs;
				}

				LWM = 1;
			}
		}
		else {
			if (!ud.srclen-- || !ud.dstlen--) {
				return false;
			}
			*ud.dst++ = *ud.src++;
			LWM = 0;
		}
	}

	return true;
}
