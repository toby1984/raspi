#include "ui.h"
#include "render.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

int run_test() 
{
  if ( ! init_render() ) {
    return 0;  
  }
  
  viewport_desc desc;
  get_viewport_desc(&desc);
  
  SDL_Rect        rectTmp;
  Uint32          nColTmp;
  for (Uint16 nPosX=0;nPosX<desc.width;nPosX++) {
          rectTmp.x = nPosX;
          rectTmp.y = desc.height/2;
          rectTmp.w = 1;
          rectTmp.h = desc.height/2;
          nColTmp = SDL_MapRGB(scrMain->format,nPosX%256,0,255-(nPosX%256));
          SDL_FillRect(scrMain,&rectTmp,nColTmp);
  }

  // Draw a green box
  Uint32 nColGreen = SDL_MapRGB(scrMain->format,0,255,0);
  SDL_Rect rectBox = {0,0,desc.width,desc.height/2};
  SDL_FillRect(scrMain,&rectBox,nColGreen);

  // Draw some text
  SDL_Color color = {255, 255, 255};
  render_text("TEST",50,50,color);

  refresh();

  SDL_Delay(3000);
  
  close_render();
  
  return 1;
}
