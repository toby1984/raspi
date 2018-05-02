#include "SDL/SDL.h"
#include <sys/time.h>
#include "log.h"
#include "input.h"
#include "render.h"

int mouseButtonPressed = 0;


int input_init_touch() {
    return 1;
}

int input_poll_touch(TouchEvent *event) 
{
 
#define FAKE_TOUCHSCREEN
#ifdef FAKE_TOUCHSCREEN
    SDL_Event test_event;
    if ( ! SDL_PollEvent(&test_event)) {
      log_debug("no events");
      return 0;    
    }
    
    switch (test_event.type) 
    {
        case SDL_MOUSEBUTTONDOWN:
            
            if ( test_event.button.button != SDL_BUTTON_LEFT ) {
              return 0;    
            }
            mouseButtonPressed = 1;
            
            event->x = test_event.button.x;
            event->y = test_event.button.y;
            event->pressure = 255;
            gettimeofday(&event->tv,NULL);
            printf("MOUSE DOWN - Button %d, Current mouse position is: (%d, %d)\n", test_event.button.button, test_event.button.x, test_event.button.y);             
            return 1;
        case SDL_MOUSEBUTTONUP:
            
            if ( test_event.button.button != SDL_BUTTON_LEFT ) {
              return 0;    
            }
            mouseButtonPressed = 0;
            printf("MOUSE UP\n");
            return 0;
        case SDL_MOUSEMOTION:
            if ( mouseButtonPressed ) 
            {
              event->x = test_event.button.x;
              event->y = test_event.button.y;
              event->pressure = 255;
              gettimeofday(&event->tv,NULL);                
              
              printf("MOUSE DRAG: (%d, %d)\n", test_event.motion.x, test_event.motion.y);
              return 1;
            }        
        default:
            return 0;
    }
#else
#error "TSLIB support not implemented yet"
#endif     
  return 1;    
}

void close_touch() {    
}
