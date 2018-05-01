#include "ui.h"
#include "render.h"
#define SDL_MAIN_HANDLED
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

void run_test_internal(void* data) 
{
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

  for( int frames = 0 ; frames < 30 * 10 ; frames++ ) 
  {
  
    SDL_Event test_event;
    while (SDL_PollEvent(&test_event)) 
    {
      switch (test_event.type) 
      {
        case SDL_MOUSEBUTTONDOWN:
          printf("We got a mouse down.\n");
          printf("Button %d, Current mouse position is: (%d, %d)\n", test_event.button.button, test_event.button.x, test_event.button.y);             
          break;          
        case SDL_MOUSEBUTTONUP:
          printf("We got a mouse up.\n");
          printf("Button %d, Current mouse position is: (%d, %d)\n", test_event.button.button, test_event.button.x, test_event.button.y);          
          break;
        case SDL_MOUSEMOTION:
          printf("We got a motion event.\n");
          printf("Current mouse position is: (%d, %d)\n", test_event.motion.x, test_event.motion.y);
          break;
        default:
          printf("Unhandled Event!\n");
          break;
      }
    }    
    SDL_Delay(32);      
  }
  SDL_Delay(3000);
}

int run_test() 
{
  if ( ! init_render() ) {
    return 0;  
  }
  
  fprintf(stdout,"Now calling run_test_internal()...\n");
  exec_on_thread(&run_test_internal,NULL,1);
  
  fprintf(stdout,"Now calling close_render()...\n");
  
  close_render();  
  
  return 1;
}
