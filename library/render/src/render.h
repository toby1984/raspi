
#ifndef RENDER_H
#define RENDER_H
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_getenv.h"

#define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"

#define RENDER_FLAG_SDL_INIT 1<<0
#define RENDER_FLAG_TTF_INIT 1<<1
#define RENDER_FLAG_TTF_FONT_LOADED 1<<2

typedef struct viewport_desc {
  int width;
  int height;
  int bitsPerPixel;
} viewport_desc;

extern SDL_Surface* scrMain;

extern int get_viewport_desc(viewport_desc *port);

extern int init_render();

extern void close_render();

extern void refresh();

extern void render_text(const char *text,int x,int y,SDL_Color color);

#endif

