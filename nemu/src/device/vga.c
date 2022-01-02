#include "common.h"

// for pa2
#ifndef HAS_IOE
#define HAS_IOE
#endif

#ifdef HAS_IOE

#include "device/map.h"
#include <SDL2/SDL.h>

#define VMEM 0xa0000000

#define SCREEN_PORT 0x100 // Note that this is not the standard
#define SCREEN_MMIO 0xa1000100
#define SYNC_PORT 0x104 // Note that this is not the standard
#define SYNC_MMIO 0xa1000104
#define SCREEN_H 300
#define SCREEN_W 400

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static uint32_t (*vmem)[SCREEN_W] = NULL;
static uint32_t *screensize_port_base = NULL;

static inline void test_vga()
{
  SDL_Surface *bmp = SDL_LoadBMP("/home/zc/ics2019/nemu/src/device/test.bmp");
  texture = SDL_CreateTextureFromSurface(renderer, bmp);
  SDL_FreeSurface(bmp);

  for (int i = 0; i < 20; i++)
  {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(100);
  }
}

static inline void update_screen()
{
  Log("update_screen called");
  static int cnt = 0;
  if ((cnt++) % 2)
  {
    SDL_Surface *bmp = SDL_LoadBMP("/home/zc/ics2019/nemu/src/device/test.bmp");
    texture = SDL_CreateTextureFromSurface(renderer, bmp);
    SDL_FreeSurface(bmp);
  }
  else
  {
    SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(vmem[0][0]));
  }
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
  SDL_Delay(100);
}

static void vga_io_handler(uint32_t offset, int len, bool is_write)
{
  Log("vga_io_handler called");
  if (is_write)
    update_screen();
}

void init_vga()
{
  char title[128];
  sprintf(title, "%s-NEMU", str(__ISA__));

  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(SCREEN_W * 2, SCREEN_H * 2, 0, &window, &renderer);
  SDL_SetWindowTitle(window, title);

#define SDL_RENDER_TEST

#ifndef SDL_RENDER_TEST

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);

#else

  test_vga();

#endif

  screensize_port_base = (void *)new_space(8);
  screensize_port_base[0] = ((SCREEN_W) << 16) | (SCREEN_H);
  add_pio_map("screen", SCREEN_PORT, (void *)screensize_port_base, 8, vga_io_handler);
  add_mmio_map("screen", SCREEN_MMIO, (void *)screensize_port_base, 8, vga_io_handler);

  vmem = (void *)new_space(0x80000);
  add_mmio_map("vmem", VMEM, (void *)vmem, 0x80000, NULL);
}
#endif /* HAS_IOE */
