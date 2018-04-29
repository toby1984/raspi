#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_getenv.h"

// #define USE_FB

// Define the primary surface for display
SDL_Surface* scrMain = NULL;

// Main entrypoint
int main(int argc, char* args[])
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
                return 1;
        }

        // Fetch the best video mode
        // - Note that the Raspberry Pi generally defaults
        //   to a 16bits/pixel framebuffer
        const SDL_VideoInfo* vInfo = SDL_GetVideoInfo();
        if (!vInfo) {
                fprintf(stderr,"ERROR in SDL_GetVideoInfo(): %s\n",SDL_GetError());
                return 1;
        }
        int     nResX = vInfo->current_w;
        int     nResY = vInfo->current_h;
        int     nDepth = vInfo->vfmt->BitsPerPixel;

        // Configure the video mode
        // - SDL_SWSURFACE appears to be most robust mode
        int     nFlags = SDL_SWSURFACE;
        scrMain = SDL_SetVideoMode(nResX,nResY,nDepth,nFlags);
        if (scrMain == 0) {
                fprintf(stderr,"ERROR in SDL_SetVideoMode(): %s\n",SDL_GetError());
                return 1;
        }

        // --------------------------------------
        // Setup TTF
        // --------------------------------------

        if (TTF_Init() < 0) {
          fprintf(stderr,"ERROR in TTF_Init(): %s\n",TTF_GetError());
          SDL_Quit();
          return 1;
        }

        const char* fontName = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
        TTF_Font* font = TTF_OpenFont(fontName, 32);
        if ( ! font ) {
          fprintf(stderr,"Failed to load TTF font %s. %s\n",fontName,TTF_GetError());
          TTF_Quit();
          SDL_Quit();
          return 1;
        }

        // --------------------------------------
        // Perform some simple drawing primitives
        // --------------------------------------

        // Draw a gradient from red to blue
        SDL_Rect        rectTmp;
        Uint32          nColTmp;
        for (Uint16 nPosX=0;nPosX<nResX;nPosX++) {
                rectTmp.x = nPosX;
                rectTmp.y = nResY/2;
                rectTmp.w = 1;
                rectTmp.h = nResY/2;
                nColTmp = SDL_MapRGB(scrMain->format,nPosX%256,0,255-(nPosX%256));
                SDL_FillRect(scrMain,&rectTmp,nColTmp);
        }

        // Draw a green box
        Uint32 nColGreen = SDL_MapRGB(scrMain->format,0,255,0);
        SDL_Rect rectBox = {0,0,nResX,nResY/2};
        SDL_FillRect(scrMain,&rectBox,nColGreen);


        // Draw some text
        SDL_Color color = {255, 255, 255};

	SDL_Surface* textSurface = TTF_RenderText_Solid(font, "TEST", color);

//	int SDL_BlitSurface(SDL_Surface*    src, const SDL_Rect* srcrect, SDL_Surface*    dst, SDL_Rect*       dstrect) 
//
        SDL_Rect srcRect = {0,0,textSurface->w,textSurface->h};
        SDL_Rect dstRect = {50,50,srcRect.w,srcRect.h};
        SDL_BlitSurface(textSurface, &srcRect, scrMain, &dstRect );

        // Now that we've completed drawing, update the main display
        SDL_Flip(scrMain);

        // Wait for a short delay
        SDL_Delay(3000);

        // close font
        TTF_CloseFont(font);

        // Close down TTF
        TTF_Quit();

        // Close down SDL
        SDL_Quit();

        return 0;
}
