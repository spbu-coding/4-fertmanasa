#include "bmp_handler.h"
#include <stdlib.h>

#define bmp_palette_size_8bpp (256 * 4)
#define header_bytes_size 54

static bmp_status BMP_LAST_ERROR_CODE = bmp_ok;

static const char* BMP_ERRORS[] = {
        "",
        "General error",
        "Could not allocate enough memory to complete the operation",
        "File input/output error",
        "File not found",
        "File is not a supported BMP variant (must be uncompressed 4, 8, 24 or 32 BPP)",
        "File is not a valid BMP image",
        "An argument is invalid or out of range",
        "The requested action is not compatible with the BMP's type"
};

const char* BMP_get_error_description() {
    if (BMP_LAST_ERROR_CODE > 0 && BMP_LAST_ERROR_CODE < bmp_error_num) {
        return BMP_ERRORS[BMP_LAST_ERROR_CODE];
    } else {
        return NULL;
    }
}

bmp1* read_BMPv3_file(char* filename) {
    bmp1* bmp;
    FILE* f;
    long int palette_size = 0;
    if (filename == NULL) {
        BMP_LAST_ERROR_CODE = bmp_invalid_argument;
        return NULL;
    }
    bmp = (bmp1*)calloc(1, sizeof(bmp1));
    if (bmp == NULL) {
        BMP_LAST_ERROR_CODE = bmp_out_of_memory;
        return NULL;
    }
    f = fopen(filename, "rb");
    if (f == NULL) {
        BMP_LAST_ERROR_CODE = bmp_file_not_found;
        free(bmp);
        return NULL;
    }
    if (read_header(bmp, f) != bmp_ok || bmp->header.magic != 0x4D42) {
        BMP_LAST_ERROR_CODE = bmp_invalid_file;
        fclose(f);
        free(bmp);
        return NULL;
    }
    if (bmp->header.bits_per_pixel == 8) {
        palette_size = bmp_palette_size_8bpp;
    }
    if ((bmp->header.bits_per_pixel != 24 && bmp->header.bits_per_pixel != 8)
        || bmp->header.compression_type != 0 || bmp->header.header_size != 40) {
        BMP_LAST_ERROR_CODE = bmp_file_not_supported;
        fclose(f);
        free(bmp);
        return NULL;
    }
    if (palette_size > 0) {
        bmp->palette = (unsigned char*)malloc(palette_size * sizeof(unsigned char));
        if (bmp->palette == NULL) {
            BMP_LAST_ERROR_CODE = bmp_out_of_memory;
            fclose(f);
            free(bmp);
            return NULL;
        }
        if (fread(bmp->palette, sizeof(unsigned char), palette_size, f) != palette_size) {
            BMP_LAST_ERROR_CODE = bmp_invalid_file;
            fclose(f);
            free(bmp->palette);
            free(bmp);
            return NULL;
        }
    } else {
        bmp->palette = NULL;
    }
    bmp->data = (unsigned char*)malloc(bmp->header.image_data_size);
    if (bmp->data == NULL) {
        BMP_LAST_ERROR_CODE = bmp_out_of_memory;
        fclose(f);
        free(bmp->palette);
        free(bmp);
        return NULL;
    }
    if (fread(bmp->data, sizeof(unsigned char), bmp->header.image_data_size, f) != bmp->header.image_data_size) {
        BMP_LAST_ERROR_CODE = bmp_invalid_file;
        fclose(f);
        free(bmp->data);
        free(bmp->palette);
        free(bmp);
        return NULL;
    }
    fclose(f);
    BMP_LAST_ERROR_CODE = bmp_ok;
    return bmp;
}

long int get_4byte_int(short first_byte_index, unsigned char* header_bytes) {
    short i = first_byte_index;
    long int x = header_bytes[i + 3] << 24 | header_bytes[i + 2] << 16 | header_bytes[i + 1] << 8 | header_bytes[i];
    return x;
}

short get_2byte_int(short first_byte_index, unsigned char* header_bytes) {
    short i = first_byte_index;
    short x = header_bytes[i + 1] << 8 | header_bytes[i];
    return x;
}

void write_4byte_hex(long int x, short first_element_index, unsigned char* array_of_bytes) {
    short i = first_element_index;
    array_of_bytes[i + 3] = (unsigned char)((x & 0xff000000) >> 24);
    array_of_bytes[i + 2] = (unsigned char)((x & 0x00ff0000) >> 16);
    array_of_bytes[i + 1] = (unsigned char)((x & 0x0000ff00) >> 8);
    array_of_bytes[i] = (unsigned char)((x & 0x000000ff) >> 0);
}

void write_2byte_hex(short x, short first_element_index, unsigned char* array_of_bytes) {
    short i = first_element_index;
    array_of_bytes[i + 1] = (unsigned char)((x & 0xff00) >> 8);
    array_of_bytes[i] = (unsigned char)((x & 0x00ff) >> 0);
}

int	read_header(bmp1* bmp, FILE* f) {
    if (bmp == NULL || f == NULL) {
        return bmp_invalid_argument;
    }
    unsigned char header_bytes[header_bytes_size];
    if (fread(header_bytes, header_bytes_size, 1, f) != 1) {
        return bmp_io_error;
    }
    bmp->header.magic = get_2byte_int(0, header_bytes);
    bmp->header.file_size  = get_4byte_int(2, header_bytes);
    bmp->header.reserved1 = get_2byte_int(6, header_bytes);
    bmp->header.reserved2 = get_2byte_int(8, header_bytes);
    bmp->header.data_offset = get_4byte_int(10, header_bytes);
    bmp->header.header_size = get_4byte_int(14, header_bytes);
    bmp->header.width = get_4byte_int(18, header_bytes);
    bmp->header.height = get_4byte_int(22, header_bytes);
    bmp->header.planes = get_2byte_int(26, header_bytes);
    bmp->header.bits_per_pixel = get_2byte_int(28, header_bytes);
    bmp->header.compression_type = get_4byte_int(30, header_bytes);
    bmp->header.image_data_size = get_4byte_int(34, header_bytes);
    bmp->header.h_pixels_per_meter = get_4byte_int(38, header_bytes);
    bmp->header.v_pixels_per_meter = get_4byte_int(42, header_bytes);
    bmp->header.colors_used = get_4byte_int(46, header_bytes);
    bmp->header.colors_required = get_4byte_int(50, header_bytes);
    return bmp_ok;
}

void write_BMPv3_file(bmp1* bmp, char* filename) {
    FILE* f;
    long int palette_size = 0;
    if (bmp->header.bits_per_pixel == 8) {
        palette_size = bmp_palette_size_8bpp;
    }
    if (filename == NULL) {
        BMP_LAST_ERROR_CODE = bmp_invalid_argument;
        return;
    }
    f = fopen(filename, "wb");
    if (f == NULL) {
        BMP_LAST_ERROR_CODE = bmp_io_error;
        return;
    }
    if (write_header(bmp, f) != bmp_ok) {
        BMP_LAST_ERROR_CODE = bmp_io_error;
        fclose(f);
        return;
    }
    if (palette_size > 0) {
        if (fwrite(bmp->palette, sizeof(unsigned char), palette_size, f) != palette_size) {
            BMP_LAST_ERROR_CODE = bmp_io_error;
            fclose(f);
            return;
        }
    }
    if (fwrite(bmp->data, sizeof(unsigned char), bmp->header.image_data_size, f) != bmp->header.image_data_size) {
        BMP_LAST_ERROR_CODE = bmp_io_error;
        fclose(f);
        return;
    }
    BMP_LAST_ERROR_CODE = bmp_ok;
    fclose(f);
}

int write_header(bmp1* bmp, FILE* f) {
    if (bmp == NULL || f == NULL) {
        return bmp_invalid_argument;
    }
    unsigned char array_of_bytes[header_bytes_size];
    write_2byte_hex(bmp->header.magic, 0, array_of_bytes);
    write_4byte_hex(bmp->header.file_size, 2, array_of_bytes);
    write_2byte_hex(bmp->header.reserved1, 6, array_of_bytes);
    write_2byte_hex(bmp->header.reserved2, 8, array_of_bytes);
    write_4byte_hex(bmp->header.data_offset, 10, array_of_bytes);
    write_4byte_hex(bmp->header.header_size, 14, array_of_bytes);
    write_4byte_hex(bmp->header.width, 18, array_of_bytes);
    write_4byte_hex(bmp->header.height, 22, array_of_bytes);
    write_2byte_hex(bmp->header.planes, 26, array_of_bytes);
    write_2byte_hex(bmp->header.bits_per_pixel, 28, array_of_bytes);
    write_4byte_hex(bmp->header.compression_type, 30, array_of_bytes);
    write_4byte_hex(bmp->header.image_data_size, 34, array_of_bytes);
    write_4byte_hex(bmp->header.h_pixels_per_meter, 38, array_of_bytes);
    write_4byte_hex(bmp->header.v_pixels_per_meter, 42, array_of_bytes);
    write_4byte_hex(bmp->header.colors_used, 46, array_of_bytes);
    write_4byte_hex(bmp->header.colors_required, 50, array_of_bytes);
    if (fwrite(array_of_bytes, header_bytes_size, 1, f) != 1) {
        return bmp_io_error;
    }
    return bmp_ok;
}

bmp_status BMP_get_error()
{
    return BMP_LAST_ERROR_CODE;
}