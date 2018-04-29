

#include "render.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_getenv.h"

// #define USE_FB
SDL_Surface* scrMain = NULL;
TTF_Font* font;
int initFlags = 0;

viewport_desc viewportInfo;

int is_initialized() {
  return initFlags == RENDER_FLAG_SDL_INIT | RENDER_FLAG_TTF_INIT | RENDER_FLAG_TTF_FONT_LOADED;
}

int get_viewport_desc(viewport_desc *port) {
  if ( is_initialized ) 
  {
    port->width = viewportInfo.width;
    port->height = viewportInfo.height;
    port->bitsPerPixel = viewportInfo.bitsPerPixel;
    return 1;  
  }
  return 0;  
}

void render_text(const char *text,int x,int y,SDL_Color color) 
{
  SDL_Surface* textSurface = TTF_RenderText_Solid(font, "TEST", color);
  SDL_Rect srcRect = {0,0,textSurface->w,textSurface->h};
  SDL_Rect dstRect = {50,50,srcRect.w,srcRect.h};
  SDL_BlitSurface(textSurface, &srcRect, scrMain, &dstRect );  
  
  SDL_FreeSurface(textSurface);
}  

int init_render() 
{
  // --------------------------------------
  // Initialization
  // --------------------------------------

#ifdef USE_FB
  // Update the environment variables for SDL to
  // work correctly with the external display on
  // LINUX frame buffer 1 (fb1).
  putenv((char*)"FRAMEBUFFER=/dev/fb1");
  putenv((char*)"SDL_FBDEV=/dev/fb1");
#endif

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr,"ERROR in SDL_Init(): %s\n",SDL_GetError());
    close_render();
    return 0;
  }
  initFlags |= RENDER_FLAG_SDL_INIT;

  // Fetch the best video mode
  // - Note that the Raspberry Pi generally defaults
  //   to a 16bits/pixel framebuffer
  const SDL_VideoInfo* vInfo = SDL_GetVideoInfo();
  if (!vInfo) {
    fprintf(stderr,"ERROR in SDL_GetVideoInfo(): %s\n",SDL_GetError());
    close_render();
    return 0;
  }
  
  viewportInfo.width = 320; // vInfo->current_w;
  viewportInfo.height = 240; // vInfo->current_h;
  viewportInfo.bitsPerPixel = 16; // vInfo->vfmt->BitsPerPixel;

  // Configure the video mode
  // - SDL_SWSURFACE appears to be most robust mode
  int     nFlags = SDL_SWSURFACE;
  scrMain = SDL_SetVideoMode(viewportInfo.width,viewportInfo.height,viewportInfo.bitsPerPixel,nFlags);
  if (scrMain == 0) {
    fprintf(stderr,"ERROR in SDL_SetVideoMode(): %s\n",SDL_GetError());
    close_render();
    return 0;
  }

  // --------------------------------------
  // Setup TTF
  // --------------------------------------

  if (TTF_Init() < 0) {
    fprintf(stderr,"ERROR in TTF_Init(): %s\n",TTF_GetError());
    close_render();
    return 0;
  }
  
  initFlags |= RENDER_FLAG_TTF_INIT;  

  font = TTF_OpenFont(FONT_PATH, 32);
  if ( ! font ) {
    fprintf(stderr,"Failed to load TTF font %s. %s\n",FONT_PATH,TTF_GetError());
    close_render();
    return 0;
  }
  initFlags |= RENDER_FLAG_TTF_FONT_LOADED;
  
  return 1;
}

void refresh() 
{
  SDL_Flip(scrMain); 
}

void close_render() 
{
    // close font
  if ( initFlags & RENDER_FLAG_TTF_FONT_LOADED) {
    TTF_CloseFont(font);
  }

  // Close down TTF
  if ( initFlags & RENDER_FLAG_TTF_INIT) {
    TTF_Quit();
  }

  // Close down SDL
  if ( initFlags & RENDER_FLAG_SDL_INIT ) {
    SDL_Quit();
  }
  initFlags = 0;
}