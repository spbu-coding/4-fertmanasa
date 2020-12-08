#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "bmp_handler.h"
#include "qdbmp.h"

#define count_of_normal_arguments 2
#define error(...) (fprintf(stderr, __VA_ARGS__))
#define bytes_amount_in_a_pixel 3
#define max_name_size_of_a_file 255
#define palette_size_8bpp (256 * 4)

int is_filename_incorrect(char* filename, char* key) {
    unsigned int filename_length = strlen(filename);
    unsigned int key_length = strlen(key);
    for (int i = 0; i < key_length; i++) {
        if (tolower(filename[filename_length - key_length + i]) != key[i]) {
            return 1;
        }
    }
    return 0;
}

typedef enum {
    mine,
    alien
} REALIZATION_TYPE;

int scan_arguments(int count_of_arguments, char** arguments, REALIZATION_TYPE* realization,
                   char* input_filename, char* output_filename) {
    if (count_of_arguments - 1 != count_of_normal_arguments) {
        error("%s", "Count of arguments must be 3");
        return 1;
    }
    if (strcmp(arguments[1], "--mine") == 0) {
        *realization = mine;
    } else if (strcmp(arguments[1], "--theirs") == 0) {
        *realization = alien;
    } else {
        error("%s", "Incorrect type of realization");
        return 1;
    }
    strcpy(input_filename, arguments[2]);
    strcpy(output_filename, arguments[3]);
    if (is_filename_incorrect(input_filename, ".bmp") || is_filename_incorrect(output_filename, ".bmp")) {
        error("%s", "File must be in bmp format");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    REALIZATION_TYPE realization;
    char input_filename[max_name_size_of_a_file];
    char output_filename[max_name_size_of_a_file];
    if (scan_arguments(argc, argv, &realization, input_filename, output_filename)) {
        return -1;
    }
    if (realization == mine) {
        bmp1* image = read_bmp_file(input_filename);
        bmp_error_checking(stderr, -2);
        if (image->header.bits_per_pixel == 24) {
            for (unsigned long int i = 0; i < image->header.width * image->header.height * bytes_amount_in_a_pixel; i++) {
                image->data[i] = ~image->data[i];
            }
        }
        else if (image->header.bits_per_pixel == 8) {
            for (int i = 0; i < palette_size_8bpp; i++) {
                if ((i + 1) % 4 != 0) {
                    image->palette[i] = ~image->palette[i];
                }
            }
        }
        else {
            error("%s", "File is not a supported BMP variant");
            return -1;
        }
        write_bmp_file(image, output_filename);
        bmp_error_checking(stderr, -1);
    } else if (realization == alien) {
        BMP* image = BMP_ReadFile(input_filename);
        bmp_error_checking(stdout, -2);
        unsigned long int width = BMP_GetWidth(image);
        unsigned long int height = BMP_GetHeight(image);
        unsigned char r, g, b;
        if (image->Header.BitsPerPixel == 24) {
            for (unsigned long int x = 0; x < width; ++x) {
                for (unsigned long int y = 0 ;y < height; ++y) {
                    BMP_GetPixelRGB(image, x, y, &r, &g, &b);
                    BMP_SetPixelRGB(image, x, y, 255 - r, 255 - g, 255 - b);
                }
            }
        } else if (image->Header.BitsPerPixel == 8) {
            for (int i = 0; i < palette_size_8bpp; i++) {
                if ((i + 1) % 4 != 0) {
                    image->Palette[i] = ~image->Palette[i];
                }
            }
        } else {
            error("%s", "File is not a supported BMP variant");
            return -1;
        }
        BMP_WriteFile(image, output_filename);
        bmp_error_checking(stdout, -1);
    }
    return 0;
}