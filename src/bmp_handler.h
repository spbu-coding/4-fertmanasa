#include <stdio.h>

#ifndef HOMEWORK_4_BMP_HANDLER_H
#define HOMEWORK_4_BMP_HANDLER_H

typedef enum {
    bmp_ok = 0,
    bmp_error,
    bmp_out_of_memory,
    bmp_io_error,
    bmp_file_not_found,
    bmp_file_not_supported,
    bmp_invalid_file,
    bmp_invalid_argument,
    bmp_type_mismatch,
    bmp_error_num
} bmp_status;

typedef struct bmp_header {
    short magic;
    long int file_size;
    short reserved1;
    short reserved2;
    long int data_offset;
    long int header_size;
    long int width;
    long int height;
    short planes;
    short bits_per_pixel;
    long int compression_type;
    long int image_data_size;
    long int h_pixels_per_meter;
    long int v_pixels_per_meter;
    long int colors_used;
    long int colors_required;
} bmp_header;

typedef struct bmp {
    bmp_header header;
    unsigned char* palette;
    unsigned char* data;
} bmp1;

bmp1* read_bmp_file(char* filename);

void write_bmp_file(bmp1* bmp, char* filename);

int write_header(bmp1* bmp, FILE* f);

int	read_header(bmp1* bmp, FILE* f);

bmp_status bmp_get_error();

const char* bmp_get_error_description();

#define bmp_error_checking(output_file, return_value) \
	if (bmp_get_error() != bmp_ok) \
	{\
		fprintf((output_file), "%s\n", bmp_get_error_description());\
		return(return_value);	\
	} \


#endif //HOMEWORK_4_BMP_HANDLER_H