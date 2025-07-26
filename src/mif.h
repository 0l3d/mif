#define MHeader "MIF_FILE"

#define MIF_COMPRESSION_LEVEL_BEST_SPEED 1
#define MIF_COMPRESSION_LEVEL_RECOMMENDED 5
#define MIF_COMPRESSION_LEVEL_BEST_COMPRESSION 9

typedef struct {
  unsigned char R, G, B;
} MColorBlock;

typedef struct {
  MColorBlock color;
} MPixel;

typedef struct {
  int width;
  int height;
  MPixel *pixels;
} MImage;

void mif_generate_pixel(MPixel *block, MColorBlock color);
void mif_write_file(const char *filenam, MImage *image, int compression,
                    int zlib_compression_level);
void mif_read_file(const char *filename, MImage *outimage);
void mif_to_ppm(const char *filename, const char *ppm_filename);
void mif_compress_file(const char *filename, const char *out_filename,
                       int compression_level);
void mif_decompress_file(const char *filename, const char *out_filename);

// TODO: void ppm_to_mif(const char *filename, MImage *outimage);
