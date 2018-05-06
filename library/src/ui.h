#ifndef UI_H
#define UI_H

#include "SDL/SDL.h"

typedef void (*ButtonHandler)(void);

int ui_add_button(char *text,SDL_Rect *bounds,ButtonHandler clickHandler);

int ui_run_test();

#endif
