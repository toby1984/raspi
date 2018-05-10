#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void buttonHandler(int buttonId) {
  printf("Button clicked >> %d <<\n",buttonId);
}

int main(int argc, char* args[])
{
  if ( mylib_init() ) 
  {
    // int mylib_add_button(char *text,int x,int y,int width,int height,ButtonHandler clickHandler) 
    int buttonId = mylib_add_button("button1",50,50,150,20,buttonHandler);
    printf("Registered button %d\n",buttonId);
    sleep(30);
    mylib_close();
    return 0;
  } 
  fprintf(stderr,"Failed to initialize library\n");
  return 1;
}
