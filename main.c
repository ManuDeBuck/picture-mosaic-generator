#include "main.h"

#include <getopt.h>

/*
 * Steps in algorithm
 * 1. Find the dimensions for the tiles depending on the input image
 * 2. Resize the input image so that the tiles fit perfectly (i.e. cut off not-used borders)
 * 3. For every image, go over all the possible places it can be located and compute it's score for that position.
 * If the computed score is better than the score of the image currently in the box, replace the image
 * 4. Do this for all the images
 */

int main(int argc, char **argv) {
  char *collage_image = NULL; // The image creating the collage for
  char *tile_directory = NULL; // The directory containing the tile image candidates
  int amount_of_tiles = 0; // The amount of tiles we're trying to fit into the collage
  int amount_of_tile_files = 0; // The amount of tile files we're trying to use in our collage
  char *output_file_name = NULL; // The file name for the output image, if left out this will be output.jpg

  int c;
  while ((c = getopt(argc, argv, "i:d:t:c:o:")) != -1) {
    switch (c) {
      case 'i':
        collage_image = (char *) malloc(sizeof(char) * strlen(optarg) + 1);
        test_allocation(collage_image);
        strcpy(collage_image, optarg);
        break;
      case 'd':
        tile_directory = (char *) malloc(sizeof(char) * strlen(optarg) + 1);
        test_allocation(tile_directory);
        strcpy(tile_directory, optarg);
        break;
      case 't':
        amount_of_tiles = atoi(optarg);
        if (amount_of_tiles < 1) {
          print_usage();
          exit(1);
        }
        break;
      case 'c':
        amount_of_tile_files = atoi(optarg);
        if (amount_of_tile_files < 1) {
          print_usage();
          exit(1);
        }
        break;
      case 'o':
        output_file_name = (char *) malloc(sizeof(char) * strlen(optarg) + 1);
        test_allocation(output_file_name);
        strcpy(output_file_name, optarg);
        break;
      default:
        print_usage();
        exit(1);
    }
  }

  if (output_file_name == NULL) {
    output_file_name = (char *) malloc(sizeof(char) * 11);
    test_allocation(output_file_name);
    strcpy(output_file_name, "output.jpg\0");
  }

  if (collage_image == NULL || tile_directory == NULL || amount_of_tiles < 1 || amount_of_tile_files < 1) {
    print_usage();
    exit(1);
  }

  // Create a collage structure
  collage *clg = malloc(sizeof(collage));
  test_allocation(clg);

  // Load the original image
  clg->original_img = stbi_load(collage_image, &(clg->original_width), &(clg->original_height), &(clg->channels), 0);

  // Compute the size of the tiles
  int total_pixels = clg->original_height * clg->original_width;
  int tile_pixel_size = (int) sqrt(total_pixels / amount_of_tiles);

  printf("Creating collage for file: %s - With (pixel) dimensions of: %d x %d\n",
         collage_image, clg->original_width, clg->original_height);
  printf("Dimensions (pixels) of a tile: %d x %d\n", tile_pixel_size, tile_pixel_size);

  // Compute the amount of tiles we will try to fit in the width and height
  int images_in_width = (int) (clg->original_width / tile_pixel_size);
  int images_in_height = (int) (clg->original_height / tile_pixel_size);

  printf("Dimensions of tiles we will fit in: %d x %d. A total of: %d\n",
         images_in_width, images_in_height, images_in_height * images_in_width);

  clg->amount_width = images_in_width;
  clg->amount_height = images_in_height;

  // The new (pixel) width and height of the collage image
  clg->new_width = images_in_width * tile_pixel_size;
  clg->new_height = images_in_height * tile_pixel_size;
  int collage_amount_pixels = clg->new_height * clg->new_width;

  clg->new_img = (unsigned char *) malloc(sizeof(char) * collage_amount_pixels * clg->channels);
  test_allocation(clg->new_img);
  // Also multiply with the amount of channels

  // Construct the score table, which will keep the scores of the current tiles in the collage
  clg->score_table = (double **) malloc(sizeof(double *) * clg->amount_height);
  test_allocation(clg->score_table);
  int i;
  for (i = 0; i < clg->amount_height; i += 1) {
    clg->score_table[i] = (double *) malloc(sizeof(double) * clg->amount_width);
    test_allocation(clg->score_table[i]);
    int j;
    for (j = 0; j < clg->amount_width; j += 1) {
      clg->score_table[i][j] = (double) -1;
    }
  }

  char *filename = (char *) malloc(sizeof(char) * (strlen(tile_directory) + amount_of_tile_files % 10 + 4));
  test_allocation(filename);

  int file_count;
  for (file_count = 0; file_count < amount_of_tile_files; file_count++) {
    update_status_bar(file_count, amount_of_tile_files, 20);

    sprintf(filename, "%s%d.jpg", tile_directory, file_count + 1);

    // Get the resized (square tile with correct pixel size)
    unsigned char *resized = resized_image_from_filename(filename, tile_pixel_size, clg->channels);

    int row;
    for (row = 0; row < clg->amount_height; row += 1) {
      int col;
      for (col = 0; col < clg->amount_width; col += 1) {
        // Compute the score for the current image and the (row, col) coordinates
        double score = score_tile_euclidean(clg, resized, col, row, tile_pixel_size);
        // If tile is empty, or new image has better (lower) score
        if (clg->score_table[row][col] == -1 || clg->score_table[row][col] > score) {
          copy_tile_to_image(clg, resized, tile_pixel_size, row, col);

          // If we have changed the image, we should adapt score accordingly
          clg->score_table[row][col] = score;
        }
      }
    }

  }
  free(filename);

  // Write the collage image
  stbi_write_jpg(output_file_name, clg->new_width, clg->new_height, clg->channels, clg->new_img, 100);

  free(collage_image);
  free(tile_directory);
  free(output_file_name);

  for (i = 0; i < clg->amount_height; i += 1) {
    free(clg->score_table[i]);
  }
  free(clg->score_table);

  stbi_image_free(clg->original_img);
  stbi_image_free(clg->new_img);

  // AAAAAAAAAAAAAND, We're done, enjoy!
  return 0;
}

/**
 * Compute the minimum of two integers
 * @param a
 * @param b
 * @return The minimum of the two
 */
int minimum(int a, int b) {
  if (a <= b) {
    return a;
  } else {
    return b;
  }
}

/**
 * Given the filename, open and resize the image
 * @param filename The filename of the image
 * @param resize_size The (pixel) size of the width and the height of the resized image
 * @param collage_channels The amount of channels of the collage image
 * @return The resized (tile) image
 */
unsigned char *resized_image_from_filename(char *filename, int resize_size, int collage_channels) {
  int width;
  int height;
  int channels;
  unsigned char *image = stbi_load(filename, &width, &height, &channels, 0);

  // Make the image a square, so the resizing won't warp the image
  int min_size = minimum(width, height);
  unsigned char *square_image = malloc(sizeof(unsigned char) * min_size * min_size * channels);
  test_allocation(square_image);

  int r;
  for (r = 0; r < min_size; r += 1) {
    int c;
    for (c = 0; c < min_size; c += 1) {
      int ch;
      for (ch = 0; ch < channels; ch += 1) {
        square_image[r * min_size * channels + c * channels + ch] = image[r * width * channels + c * channels + ch];
      }
    }
  }

  // Resize the square image to the needed resolution
  unsigned char *resized = malloc(sizeof(unsigned char) * resize_size * resize_size * channels);
  test_allocation(resized);
  stbir_resize_uint8(square_image, min_size, min_size, 0, resized, resize_size, resize_size, 0, channels);

  stbi_image_free(image);
  stbi_image_free(square_image);

  return resized;
}

/**
 * Compute the score for a tile in the collage depending on the original image, using the euclidean distance
 * @param collage The collage structure
 * @param tile_image The tile we want to compute the score for
 * @param col The col we want to place the tile in the grid
 * @param row The row "  "    "  "     "   "    "  "   "
 * @param tile_size The (pixel) size of the tile
 * @return
 */
double score_tile_euclidean(collage *collage, unsigned char *tile_image, int col, int row, int tile_size) {
  double score = 0;

  int i;
  for (i = 0; i < tile_size; i += 1) {
    int j;
    for (j = 0; j < tile_size; j += 1) {
      int index_original = row * tile_size * collage->original_width * collage->channels +
                           col * tile_size * collage->channels;
      int index_tile = i * tile_size * collage->channels + j * collage->channels;
      // Compute the euclidean distance between the two images
      score += pow(collage->original_img[index_original] - tile_image[index_tile], 2) +
               pow(collage->original_img[index_original + 1] - tile_image[index_tile + 1], 2) +
               pow(collage->original_img[index_original + 2] - tile_image[index_tile + 2], 2);
    }
  }

  // the lower the score, the similar the images are
  return score;
}

/**
 * Test if the allocation succeeded. If not, fail and exit.
 * @param el The element we want to test
 */
void test_allocation(void *el) {
  if (el == NULL) {
    printf("Allocation failed.");
    exit(1);
  }
}

/**
 * Print the usage.
 */
void print_usage() {
  printf("USAGE: ./Collage -i [path_to_image] -d [path_to_tile_directory] -t [amount_of_tiles] -c [amount_of_tile_files] -o [output_name]\n");
  printf("    [path_to_image]:          path to the (jpg) image you want to create a collage for\n");
  printf("    [path_to_tile_directory]: path to the directory that holds all the images that can be used as tiles for the collage\n");
  printf("    [amount_of_tiles]:        the amount of tiles the program will **try** to fit in, this will vary depending on the input image\n");
  printf("    [amount_of_tile_files]:   the amount of files the program should try from the tile_directory; note that all the files should be numbered incrementally\n");
  printf("    [output_name]:            the name you want to give to your output file; if left blank output.jpg will be used\n");
}

/**
 * Copy the tile to the row and col in the collage
 * @param clg The collage structure
 * @param image The tile image
 * @param tile_pixel_size The pixel size of the tile
 * @param col Column
 * @param row Row
 */
void copy_tile_to_image(collage *clg, unsigned char *image, int tile_pixel_size, int row, int col) {
  int rowindex;
  for (rowindex = 0; rowindex < tile_pixel_size; rowindex += 1) {
    int colindex;
    for (colindex = 0; colindex < tile_pixel_size; colindex += 1) {
      int channel;
      for (channel = 0; channel < clg->channels; channel += 1) {
        int index = (row * tile_pixel_size) * clg->new_width * clg->channels +
                    (col * tile_pixel_size) * clg->channels +
                    rowindex * clg->new_width * clg->channels + colindex * clg->channels;

        clg->new_img[index + channel] =
                image[rowindex * tile_pixel_size * clg->channels + colindex * clg->channels + channel];
      }
    }
  }
}

/**
 * Little function that prints a status bar instead of printing 10203123123 lines with the number...
 * @param index The current index we're working on
 * @param total The total of tries
 * @param length_bar The length we want the bar to have
 */
void update_status_bar(int index, int total, int length_bar) {
  if (index > 0) {
    printf("\r"); // Little hack to remove the last printed line
  }
  printf("Progress: [");

  int amount_bars = (int) total / length_bar;
  int bars = (int) index / amount_bars;

  int i;
  for (i = 0; i < length_bar; i++) {
    if (i <= bars) {
      printf("\u2588");
    } else {
      printf(" ");
    }
  }

  printf("] %.2f%%", ((float) index / (float) total) * 100.0);
}
