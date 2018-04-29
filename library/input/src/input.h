#ifndef INPUT_H
#define INPUT_H

typedef struct TouchEvent {
        int             x;
        int             y;
        unsigned int    pressure;
        struct timeval  tv;
} TouchEvent;

extern int init_touch();

extern int poll_touch(TouchEvent *event);

extern void close_touch();

#endif