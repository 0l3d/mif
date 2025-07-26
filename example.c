#include "src/mif.h"
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

int main() {
  MImage image;
  image.width = 1440;
  image.height = 720;
  image.pixels = malloc(sizeof(MPixel) * image.width * image.height);

  MColorBlock red = {255, 0, 0};
  for (int i = 2; i < image.width * image.height; i++) {
    mif_generate_pixel(&image.pixels[i], red);
  }

  int compression = 1;
  mif_write_file("example.mif", &image, compression, Z_BEST_COMPRESSION);

  MImage read_image;
  mif_read_file("example.mif", &read_image);

  printf("Width: %d Height: %d\n", read_image.width, read_image.height);

  free(image.pixels);
  free(read_image.pixels);
}
