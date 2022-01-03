#include <am.h>
#include <amdev.h>
#include <nemu.h>

#define min(x, y) ((x) < (y)) ? (x) : (y)

size_t __am_video_read(uintptr_t reg, void *buf, size_t size)
{
  switch (reg)
  {
  case _DEVREG_VIDEO_INFO:
  {
    _DEV_VIDEO_INFO_t *info = (_DEV_VIDEO_INFO_t *)buf;

    // the height and width are both short number, combined by one uint32
    uint32_t screen_info = inl(SCREEN_ADDR);

    info->width = (screen_info >> 16) & 0x0000ffff;
    info->height = screen_info & 0x0000ffff;

    return sizeof(_DEV_VIDEO_INFO_t);
  }
  }
  return 0;
}

size_t __am_video_write(uintptr_t reg, void *buf, size_t size)
{
  switch (reg)
  {
  case _DEVREG_VIDEO_FBCTL:
  {
    _DEV_VIDEO_FBCTL_t *ctl = (_DEV_VIDEO_FBCTL_t *)buf;

    int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
    uint32_t *pixels = ctl->pixels;

    int W = screen_width();
    int H = screen_height();
    int cp_bytes = sizeof(uint32_t) * min(w, W - x);
    uint32_t *fb = (uint32_t *)FB_ADDR;
    for (int j = 0; j < h && y + j < H; j++)
    {
      memcpy(&fb[(y + j) * W + x], pixels, cp_bytes);
      pixels += w;
    }

    if (ctl->sync)
    {
      printf ("sync video..., address: %x\n", SYNC_ADDR);
      outl(SYNC_ADDR, 1);
    }
    return size;
  }
  }
  return 0;
}

void __am_vga_init()
{

// #define DEBUG_VGA

#ifdef DEBUG_VGA
  int i;
  int size = screen_width() * screen_height();
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < size; i++)
    fb[i] = 0;
  draw_sync();
#endif // DEBUG
}