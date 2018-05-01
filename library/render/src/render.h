
#ifndef RENDER_H
#define RENDER_H
#include "SDL/SDL.h"

#define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"

// #define USE_FB

#define RENDER_FLAG_SDL_INIT 1<<0
#define RENDER_FLAG_TTF_INIT 1<<1
#define RENDER_FLAG_TTF_FONT_LOADED 1<<2

typedef struct viewport_desc {
  int width;
  int height;
  int bitsPerPixel;
} viewport_desc;

typedef void (*RenderCallback)(void*);

int exec_on_thread(RenderCallback callback,void *data,int awaitCompletion);

SDL_Surface* scrMain;

int get_viewport_desc(viewport_desc *port);

int init_render();

void close_render();

void render_text(const char *text,int x,int y,SDL_Color color);

#endif

