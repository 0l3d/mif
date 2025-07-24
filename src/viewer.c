#include "mif.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("You must enter an image path.\n");
    return 1;
  }

  char *image_path = argv[1];
  MImage image;
  mif_read_file(image_path, &image);

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL init failed: %s",
                 SDL_GetError());
    return 2;
  }

  SDL_Window *window;
  SDL_Renderer *renderer;
  if (SDL_CreateWindowAndRenderer(image.width, image.height,
                                  SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "SDL CreateWindowAndRenderer failed: %s", SDL_GetError());
    SDL_Quit();
    return 3;
  }

  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(renderer);

  for (int y = 0; y < image.height; y++) {
    for (int x = 0; x < image.width; x++) {
      int i = y * image.width + x;
      SDL_SetRenderDrawColor(renderer, image.pixels[i].color.R,
                             image.pixels[i].color.G, image.pixels[i].color.B,
                             255);
      SDL_RenderDrawPoint(renderer, x, y);
    }
  }

  SDL_RenderPresent(renderer);

  SDL_Event event;
  int running = 1;
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      }
    }
    SDL_Delay(10);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  free(image.pixels);

  return 0;
}
