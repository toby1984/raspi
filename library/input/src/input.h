#ifndef INPUT_H
#define INPUT_H

#define USE_MOUSE

enum TouchEventType { CLICK, DRAG };

typedef struct TouchEvent {
  int x;
  int y;
  int type; // TouchEventType
} TouchEvent;

extern int init_input();

extern void close_input();

extern int read_events(TouchEvent *buffer,int bufferSize);

#endif