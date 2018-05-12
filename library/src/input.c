#include "SDL/SDL.h"
#include <sys/time.h>
#include "log.h"
#include "input.h"
#include "render.h"

static int mouseButtonPressed = 0;

static volatile InputHandler inputHandler = NULL;

static volatile struct timeval lastTouchEvent={0,0};

int input_init_touch(void) {
    return 1;
}

int input_poll_touch(TouchEvent *event) 
{
 
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
            if ( mouseButtonPressed == 0 ) 
            {
                log_info("button #1 pressed");              
                mouseButtonPressed = 1;
                
                event->x = test_event.button.x;
                event->y = test_event.button.y;
                event->pressure = 255;            
                gettimeofday(&event->tv,NULL);
                event->type = TOUCH_START;
                
                printf("MOUSE DOWN - Button %d, Current mouse position is: (%d, %d)\n", test_event.button.button, test_event.button.x, test_event.button.y);             
                return 1;
            }
            return 0;
        case SDL_MOUSEBUTTONUP:
            
            if ( test_event.button.button != SDL_BUTTON_LEFT ) {
              return 0;    
            }
            if ( mouseButtonPressed ) 
            {
                log_info("button #1 released");               
              event->x = test_event.button.x;
              event->y = test_event.button.y;
              event->pressure = 0;            
              gettimeofday(&event->tv,NULL);                        
              event->type = TOUCH_STOP;    
            
              mouseButtonPressed = 0;
              printf("MOUSE UP\n");
              return 1;
            } 
            return 0;
        case SDL_MOUSEMOTION:
            if ( mouseButtonPressed ) 
            {
              event->x = test_event.button.x;
              event->y = test_event.button.y;
              event->pressure = 255;
              gettimeofday(&event->tv,NULL);                
              event->type = TOUCH_CONTINUE; 
              
              printf("MOUSE DRAG: (%d, %d)\n", test_event.motion.x, test_event.motion.y);
              return 1;
            }        
        default:
            return 0;
    }
#else
#error "TSLIB support not implemented yet"
//           event->x = test_event.button.x;
//           event->y = test_event.button.y;
//           event->pressure = 255;            
//           gettimeofday(&event->tv,NULL);
//           
//           if ( lastTouchEvent.tv_sec == 0 && lastTouchEvent.tv_usec == 0 ) {
//               event->type = TouchEventType.TOUCH_START;
//           } else {
//              struct timeval elapsed; 
//              // timersub() subtracts the time value in b from the time value in a, and places the result in the timeval pointed to by res. 
//              //  void timersub(struct timeval *a/end, struct timeval *b/start, struct timeval *res);
//              timersub(&event->tv,&lastTouchEvent,&elapsed);    
//              int elapsedMillis = elapsed.tv_sec*1000 + elapsed.tv_usec/1000;
//              if ( elapsedMillis > TOUCH_STOP_DELAY_MILLIS ) {
//                event->type = TouchEventType.TOUCH_STOP;    
//              } else {
//                event->type = TouchEventType.TOUCH_CONTINUE;  
//              }
//           }
#endif     
  return 1;    
}

void input_close_touch(void) {    
}

void input_set_input_handler(InputHandler handler) {
  inputHandler = handler;
  __sync_synchronize(); 
}

void input_invoke_input_handler(TouchEvent *event) {
  log_debug("Touch event at (%d,%d)",event->x,event->y);
  __sync_synchronize();
 InputHandler handler=inputHandler; 
  if ( handler != NULL ) {
    handler(event);    
  }
}