#include "ui.h"
#include "input.h"
#include "render.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include <stdio.h>
#include "log.h"
#include <unistd.h>

/*
 * typedef struct button_desc {
    SDL_Rect bounds;
    SDL_Color borderColor;
    SDL_Color backgroundColor;
    SDL_Color textColor;
    char *text;
} button_desc;
 */
int render_button(char *text,SDL_Rect bounds)  {
    button_desc desc;
    desc.bounds = bounds;
    desc.borderColor = {255,255,255,128};
    desc.backgroundColor = {128,128,128,128};
    desc.textColor = {255,255,255,128};
    desc.text = text;
    
    return render_draw_button(&desc);
}

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

  SDL_Rect buttonBounds = {50,50,150,20};
  render_button("button1",&buttonBounds);
  
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
