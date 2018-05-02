#ifndef INPUT_H
#define INPUT_H

#include <sys/time.h>

#define FAKE_TOUCHSCREEN

typedef struct TouchEvent {
        int             x;
        int             y;
        unsigned int    pressure;
        struct timeval  tv;
} TouchEvent;

int input_init_touch();

int input_poll_touch(TouchEvent *event);

void input_close_touch();

#endif
