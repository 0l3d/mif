#include "mif.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#define CHUNK 16384

void mif_generate_pixel(MPixel *block, MColorBlock color) {
  block->color = color;
}

void mif_write_file(const char *filename, MImage *image, int compression,
                    int zlib_compression_level) {
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
  const char *temp_file = "example.mif.tmp";
  mif_compress_file(filename, temp_file, zlib_compression_level);
  rename(temp_file, filename);
}

void mif_read_file(const char *filename, MImage *outimage) {
  const char *temp_file = "temp_uncompressed.mif";
  mif_decompress_file(filename, temp_file);
  FILE *mif_file = fopen(temp_file, "rb");
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
  remove(temp_file);
}

void mif_to_ppm(const char *filename, const char *ppm_filename) {
  MImage image;
  mif_read_file(filename, &image);
  FILE *ppm_file = fopen(ppm_filename, "wb");
  if (!ppm_file) {
    perror("PPM open error");
    return;
  }

  fprintf(ppm_file, "P6\n%d %d\n255\n", image.width, image.height);
  for (int i = 0; i < image.width * image.height; i++) {
    fwrite(&image.pixels[i].color.R, 1, 1, ppm_file);
    fwrite(&image.pixels[i].color.G, 1, 1, ppm_file);
    fwrite(&image.pixels[i].color.B, 1, 1, ppm_file);
  }
  fclose(ppm_file);
  free(image.pixels);
}

void mif_compress_file(const char *filename, const char *out_filename,
                       int compression_level) {
  FILE *infile = fopen(filename, "rb");
  if (!infile) {
    perror("Input file open failed");
    return;
  }

  FILE *outfile = fopen(out_filename, "wb");
  if (!outfile) {
    perror("Output file open failed");
    fclose(infile);
    return;
  }
  int ret, flush;
  unsigned have;
  z_stream stream;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  ret = deflateInit(&stream, compression_level);
  if (ret != Z_OK)
    return;

  do {
    stream.avail_in = fread(in, 1, CHUNK, infile);
    if (ferror(infile)) {
      (void)deflateEnd(&stream);
      return;
    }
    flush = feof(infile) ? Z_FINISH : Z_NO_FLUSH;
    stream.next_in = in;

    do {
      stream.avail_out = CHUNK;
      stream.next_out = out;
      ret = deflate(&stream, flush);
      assert(ret != Z_STREAM_ERROR);
      have = CHUNK - stream.avail_out;
      if (fwrite(out, 1, have, outfile) != have || ferror(outfile)) {
        (void)deflateEnd(&stream);
        return;
      }
    } while (stream.avail_out == 0);
    assert(stream.avail_in == 0);
  } while (flush != Z_FINISH);
  assert(ret == Z_STREAM_END);

  (void)deflateEnd(&stream);
  fclose(infile);
  fclose(outfile);
}

void mif_decompress_file(const char *filename, const char *out_filename) {
  FILE *infile = fopen(filename, "rb");
  FILE *outfile = fopen(out_filename, "wb");

  int ret;
  unsigned have;
  z_stream stream;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  ret = inflateInit(&stream);
  if (ret != Z_OK)
    return;

  do {
    stream.avail_in = fread(in, 1, CHUNK, infile);
    if (ferror(infile)) {
      (void)inflateEnd(&stream);
      return;
    }
    if (stream.avail_in == 0)
      break;
    stream.next_in = in;

    do {
      stream.avail_out = CHUNK;
      stream.next_out = out;
      ret = inflate(&stream, Z_NO_FLUSH);
      assert(ret != Z_STREAM_ERROR);
      switch (ret) {
      case Z_NEED_DICT:
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        (void)inflateEnd(&stream);
        return;
      }
      have = CHUNK - stream.avail_out;
      if (fwrite(out, 1, have, outfile) != have || ferror(outfile)) {
        (void)inflateEnd(&stream);
        return;
      }
    } while (stream.avail_out == 0);
  } while (ret != Z_STREAM_END);

  (void)inflateEnd(&stream);
  fclose(infile);
  fclose(outfile);
}
