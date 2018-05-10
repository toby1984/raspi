#ifndef INPUT_H
#define INPUT_H

#include <sys/time.h>

#define FAKE_TOUCHSCREEN

// time in milliseconds after
// which we assume that the user
// stopped touching
#define TOUCH_STOP_DELAY_MILLIS 10

enum TouchEventType { TOUCH_START,TOUCH_CONTINUE,TOUCH_STOP };

typedef struct TouchEvent {
        int             x;
        int             y;
        unsigned int    pressure;
        enum TouchEventType type;
        struct timeval  tv;
} TouchEvent;

typedef void (*InputHandler)(TouchEvent*);

void input_set_input_handler(InputHandler handler);

void input_invoke_input_handler(TouchEvent *event);

int input_init_touch(void);

int input_poll_touch(TouchEvent *event);

void input_close_touch(void);

#endif
