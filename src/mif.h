#define MHeader "MIF_FILE"

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
void mif_write_file(const char *filenam, MImage *image, int compression);
void mif_read_file(const char *filename, MImage *outimage);
void mif_to_ppm(const char *filename, const char* ppm_filename);
void ppm_to_mif(const char *filename, MImage *outimage);
