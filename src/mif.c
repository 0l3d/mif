#include "mif.h"
#include <stdio.h>
#include <stdlib.h>

void mif_generate_pixel(MPixel *block, MColorBlock color) {
  block->color = color;
}

void mif_write_file(const char *filename, MImage *image, int compression) {
  FILE *mif_file = fopen(filename, "wb");
  if (mif_file == NULL) {
    perror("mif_write_file failed to open file: ");
    return;
  }
  char header[] = MHeader;
  fwrite(header, 1, 8, mif_file);
  fwrite(&compression, 1, 1, mif_file);
  fwrite(&image->width, sizeof(int), 1, mif_file);
  fwrite(&image->height, sizeof(int), 1, mif_file);

  if (compression == 1) {
    for (int i = 0; i < image->height; i++) {
      int run_len = 1;
      MColorBlock prev_color = image->pixels[i * image->width].color;
      for (int j = 1; j < image->width; j++) {
        MColorBlock color = image->pixels[i * image->width + j].color;

        if (color.B == prev_color.B && color.G == prev_color.G &&
            color.R == prev_color.R) {
          run_len++;
        } else {
          fwrite(&run_len, sizeof(run_len), 1, mif_file);
          fwrite(&prev_color.R, 1, 1, mif_file);
          fwrite(&prev_color.G, 1, 1, mif_file);
          fwrite(&prev_color.B, 1, 1, mif_file);
          run_len = 1;
          prev_color = color;
        }
      }
      fwrite(&run_len, sizeof(run_len), 1, mif_file);
      fwrite(&prev_color.R, 1, 1, mif_file);
      fwrite(&prev_color.G, 1, 1, mif_file);
      fwrite(&prev_color.B, 1, 1, mif_file);
    }
  } else if (compression == 0) {
    for (int i = 0; i < image->height * image->width; i++) {
      fwrite(&image->pixels[i].color.R, 1, 1, mif_file);
      fwrite(&image->pixels[i].color.G, 1, 1, mif_file);
      fwrite(&image->pixels[i].color.B, 1, 1, mif_file);
    }
  } else {
    fprintf(stderr, "Unrecognized compression_flag: %d", compression);
    return;
  }
  fclose(mif_file);
}

void mif_read_file(const char *filename, MImage *outimage) {
  FILE *mif_file = fopen(filename, "rb");
  if (mif_file == NULL) {
    perror("Image open error.");
    return;
  }

  char header[9];
  fread(header, 1, 8, mif_file);
  header[8] = '\0';

  unsigned char compression_flag;

  fread(&compression_flag, 1, 1, mif_file);
  int width, height;
  fread(&width, sizeof(int), 1, mif_file);
  fread(&height, sizeof(int), 1, mif_file);
  outimage->width = width;
  outimage->height = height;
  outimage->pixels = malloc(sizeof(MPixel) * width * height);
  if (!outimage->pixels) {
    perror("memory alloc failed");
    fclose(mif_file);
    return;
  }
  if (compression_flag == 1) {
    int x = 0;
    while (x < width * height) {
      int runlen;
      unsigned char r, g, b;

      if (fread(&runlen, sizeof(runlen), 1, mif_file) != 1)
        break;
      if (fread(&r, 1, 1, mif_file) != 1)
        break;
      if (fread(&g, 1, 1, mif_file) != 1)
        break;
      if (fread(&b, 1, 1, mif_file) != 1)
        break;

      for (int i = 0; i < runlen && x < width * height; i++) {
        outimage->pixels[x].color.R = r;
        outimage->pixels[x].color.G = g;
        outimage->pixels[x].color.B = b;
        x++;
      }
    }
  } else if (compression_flag == 0) {
    for (int i = 0; i < width * height; i++) {
      fread(&outimage->pixels[i].color.R, 1, 1, mif_file);
      fread(&outimage->pixels[i].color.G, 1, 1, mif_file);
      fread(&outimage->pixels[i].color.B, 1, 1, mif_file);
    }
  } else {
    fprintf(stderr, "Unrecognized compression flag: %d", compression_flag);
  }
  fclose(mif_file);
}
