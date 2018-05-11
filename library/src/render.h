
#ifndef RENDER_H
#define RENDER_H

#include "SDL/SDL.h"
#include "ui.h"

#define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"

// #define USE_FB

#define RENDER_FLAG_SDL_INIT (1<<0)
#define RENDER_FLAG_TTF_INIT (1<<1)
#define RENDER_FLAG_TTF_FONT_LOADED (1<<2)
#define RENDER_FLAG_PNG_INITIALIZED (1<<3)

#define FONT_SIZE 16

#define LISTVIEW_ITEM_HEIGHT 20

extern SDL_Surface* scrMain;

typedef struct viewport_desc {
  int width;
  int height;
  int bitsPerPixel;
} viewport_desc;

typedef struct button_desc 
{   
    SDL_Rect bounds;
    SDL_Color borderColor;
    SDL_Color backgroundColor;
    SDL_Color textColor;
    SDL_Color clickedColor;    
    int cornerRadius;
    int roundedCorners;
    int fontSize;
    int pressed;
    char *text;
    SDL_Surface *image;
} button_desc;

typedef void* (*RenderCallback)(void*);

void *render_exec_on_thread(RenderCallback callback,void *data,int awaitCompletion);

int render_get_viewport_desc(viewport_desc *port);

void render_render_text(const char *text,int x,int y,SDL_Color color);

int render_init_render(void);

void render_close_render(void);

void render_text(const char *text,int x,int y,SDL_Color color);

int render_has_error(void);

int render_draw_listview(listview_entry *listView);

volatile const char* render_get_error(void);

int render_draw_button(button_desc *button);

SDL_Surface *render_load_image(char *file);

void render_free_surface(SDL_Surface *surface);
#endif

