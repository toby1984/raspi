#include "mylib.h"
#include "ui.h"

int mylib_add_button(char *text,int x,int y,int width,int height,ButtonHandler clickHandler) 
{
  SDL_Rect rect = {x,y,width,height};
  return ui_add_button(text,&rect,clickHandler);  
}

int mylib_init(void) {
  return ui_init();  
}

void mylib_close(void) {
  ui_close();  
}
  