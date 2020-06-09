#ifndef COLLAGE_MAKER_MAIN_H
#define COLLAGE_MAKER_MAIN_H

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include "stb_image/stb_image_resize.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image/stb_image_write.h"

typedef struct collage {
    unsigned char *original_img; // Original image data
    int channels; // The amount of data channels for the original image
    int original_width; // Width and height of original image
    int original_height;

    unsigned char *new_img; // The collage image data
    int new_width; // the width and height for the new image (so that collage-images can be square)
    int new_height;
    int amount_width; // Amount of images that are in width and height of new image
    int amount_height;

    double **score_table; // Score tabel, for each of the amount_width * amount_height this table keeps
    // the score of the current image in the field, this can be used to check if the new score is better
    // and the image should thus be changed
} collage;

unsigned char *resized_image_from_filename(char *filename, int resize_size, int collage_channels);

double score_tile_euclidean(collage *collage, unsigned char *tile_image, int col, int row, int tile_size);

void test_allocation(void *el);

void print_usage();

void copy_tile_to_image(collage* clg, unsigned char* image, int tile_pixel_size, int row, int col);

void update_status_bar(int index, int total, int length_bar);

#endif //COLLAGE_MAKER_MAIN_H
