  #include "mylib.h"
  #include <stdio.h>
  #include "SDL/SDL.h"

  int main(int argc, char* args[])
  {

//     sdl_init_test();
    
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr,"ERROR in SDL_Init(): %s\n",SDL_GetError());
    return;
  }
  fprintf(stdout,"SDL_Init() success\n");  
  SDL_Quit();
  
  //  return do_test();
  }
