# MIF

MIF is a simple image format with compression features.

## Features

- MIF now has its own viewer.
- RLE Compression
- ZLIB Compression
- All visual information is stored as bytes.

# Usage

**For viewer and example.c**:

```bash
git clone https://github.com/0l3d/mif.git
cd mif/
make # for viewer
gcc src/mif.c example.c -o example -lz # for example
```

The example file produces a 1440x720 mif file consisting entirely of red pixels.
If you run the example file, a file named `example.mif` will be created.

```
./example
```

# API

## General

- `MColorBlock`: A color block containing 3 RGB colors.
- `MPixel`: A pixel containing a color block.
- `MImage`: Image containing pixels, height and width.
- `mif_generate_pixel`: High level interface for generating a pixel.
- `mif_write_file`: a function that takes a filename, an MImage, and flag arguments for RLE Compression and produces an image file
- `mif_read_file`: Function that reads data from a `.mif` file and saves it to MImage.

## Extra

- `mif_to_ppm`: It allows you to convert your mif files to ppm format (remember, an average mif file will be many times more efficient than a ppm file. For example, if you have a 1920x1080 mif file consisting entirely of red pixels, it will usually take up less than 100 bytes, but if you convert this mif file, which takes up less than 100 bytes, to ppm, it will most likely take up a size of 6 MB.)

- `mif_compress_file`: Compress any file with zlib (it does not have to be a mif file).

- `mif_decompress_file`: Decompress any file you compressed with zlib (it doesn't have to be a mif file).

## Examples

### Write Data

```c
MImage image;
image.width = 7;
image.height = 1;
image.pixels = malloc(sizeof(MPixel) * image.width);

MColorBlock red = {255, 0, 0};
MColorBlock black = {0, 0, 0};

mif_generate_pixel(&image.pixels[0], red);
mif_generate_pixel(&image.pixels[1], red);
for (int i = 2; i < 7; i++) {
  mif_generate_pixel(&image.pixels[i], black);
}

int compression = 1;
mif_write_file("example.mif", &image, compression);
```

### Read Data

```c
MImage read_image;
mif_read_file("example.mif", &read_image);

printf("Width: %d Height: %d\n", read_image.width, read_image.height);
for (int i = 0; i < read_image.width * read_image.height; i++) {
  printf("Pixel %d: R=%d G=%d B=%d\n", i, read_image.pixels[i].color.R,
        read_image.pixels[i].color.G, read_image.pixels[i].color.B);
}
```

### DONT FORGET

```c
free(image.pixels);
free(read_image.pixels);
```

## LICENSE

This project is licensed under the **GPL-3.0 License**.

## Author

Created By **0l3d**.
