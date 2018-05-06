
#ifndef RENDER_H
#define RENDER_H

#include "SDL/SDL.h"

#define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"

// #define USE_FB

#define RENDER_FLAG_SDL_INIT 1<<0
#define RENDER_FLAG_TTF_INIT 1<<1
#define RENDER_FLAG_TTF_FONT_LOADED 1<<2

#define FONT_SIZE 16

extern SDL_Surface* scrMain;

typedef struct viewport_desc {
  int width;
  int height;
  int bitsPerPixel;
} viewport_desc;

typedef struct button_desc {
    SDL_Rect bounds;
    SDL_Color borderColor;
    SDL_Color backgroundColor;
    SDL_Color textColor;
    int fontSize;
    char *text;
} button_desc;

typedef int (*RenderCallback)(void*);

int render_exec_on_thread(RenderCallback callback,void *data,int awaitCompletion);

int render_get_viewport_desc(viewport_desc *port);

void render_render_text(const char *text,int x,int y,SDL_Color color);

int render_init_render();

void render_close_render();

void render_text(const char *text,int x,int y,SDL_Color color);

void sdl_init_test2();

int render_has_error();

volatile const char* render_get_error();

int render_draw_button(button_desc *button);

#endif

