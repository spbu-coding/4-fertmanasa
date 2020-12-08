#include <stdio.h>
#include <string.h>
#include "bmp_handler.h"
#include <math.h>
#include <stdlib.h>

#define count_of_normal_arguments 2
#define error(...) (fprintf(stderr, __VA_ARGS__))
#define bytes_amount_in_a_pixel 3
#define max_name_size_of_a_file 255
#define bmp_palette_size_8bpp (256 * 4)

int compare_images(bmp1* image1, bmp1* image2) {
    if (image1->header.bits_per_pixel != image2->header.bits_per_pixel) {
        error("%s", "Images must be of the same bitness");
        return -1;
    }
    if (image1->header.width != image2->header.width || abs(image1->header.height) != abs(image2->header.height)) {
        error("%s", "Images must be equal size");
        return -1;
    }
    int width = image1->header.width;
    int height = abs(image1->header.height);
    int bits_per_pixel = image1->header.bits_per_pixel;
    int count_diff = 0;
    if (bits_per_pixel == 8) {
        for (int i = 0; i < bmp_palette_size_8bpp; i++) {
            if (image1->palette[i] != image2->palette[i]) {
                error("%s", "Images have different palettes");
                return 0;
            }
        }
        if (image1->header.height * image2->header.height > 0) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    if (image1->data[y * width + x] != image2->data[y * width + x]) {
                        error("%d %d\n", x, y);
                        count_diff++;
                    }
                    if (count_diff == 100) {
                        return 0;
                    }
                }
            }
        } else {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    if (image1->data[(height - y - 1) * width + x] != image2->data[y * width + x]) {
                        error("%d %d\n", x, y);
                        count_diff++;
                    }
                    if (count_diff == 100) {
                        return 0;
                    }
                }
            }
        }
    } else if (bits_per_pixel == 24) {
        if (image1->header.height * image2->header.height > 0) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    //checking red, green, blue values
                    if (image1->data[3 * (y * width + x)] != image2->data[3 * (y * width + x)] ||
                        image1->data[3 * (y * width + x) + 1] != image2->data[3 * (y * width + x) + 1] ||
                        image1->data[3 * (y * width + x) + 2] != image2->data[3 * (y * width + x) + 2]) {
                        error("%d %d\n", x, y);
                        count_diff++;
                    }
                    if (count_diff == 100) {
                        return 0;
                    }
                }
            }
        } else {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    //checking red, green, blue values
                    if (image1->data[3 * ((height - y - 1) * width + x)] != image2->data[3 * (y * width + x)] ||
                        image1->data[3 * ((height - y - 1) * width + x) + 1] != image2->data[3 * (y * width + x) + 1] ||
                        image1->data[3 * ((height - y - 1) * width + x) + 2] != image2->data[3 * (y * width + x) + 2]) {
                        error("%d %d\n", x, height - y - 1);
                        count_diff++;
                    }
                    if (count_diff == 100) {
                        return 0;
                    }
                }
            }
        }

    }
    return 0;
}

int scan_arguments(int count_of_arguments, char** arguments,
                   char* filename1, char* filename2) {
    if (count_of_arguments - 1 != count_of_normal_arguments) {
        error("%s", "Count of arguments must be 2");
        return 0;
    }
    strcpy(filename1, arguments[1]);
    strcpy(filename2, arguments[2]);
    return 1;
}

int main(int argc, char* argv[]) {
    char input_filename1[max_name_size_of_a_file];
    char input_filename2[max_name_size_of_a_file];
    if (!scan_arguments(argc, argv, input_filename1, input_filename2)) {
        return -1;
    }
    bmp1* image1 = read_bmp_file(input_filename1);
    bmp_error_checking(stderr, -2);
    bmp1* image2 = read_bmp_file(input_filename2);
    bmp_error_checking(stderr, -2);
    if (compare_images(image1, image2)) {
        return -1;
    }
    return 0; 
}