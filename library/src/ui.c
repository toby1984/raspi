#include "ui.h"
#include "input.h"
#include "render.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include <stdio.h>
#include "log.h"
#include <unistd.h>

int ui_run_test_internal(void* data) 
{
  viewport_desc desc;
  render_get_viewport_desc(&desc);
  
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
  render_render_text("TEST",50,50,color);
  return 1;
}

int ui_run_test() 
{
  if ( ! render_init_render() ) {
    return 0;  
  }
  
  log_debug("Now calling run_test_internal()...");
  render_exec_on_thread(&ui_run_test_internal,NULL,0);
  
  log_debug("Now sleeping 6 seconds ...");  
  sleep(6);
  log_debug("Now calling close_render()...");
  
  render_close_render();  
  
  return 1;
}
