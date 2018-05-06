#ifndef UI_H
#define UI_H

#include "SDL/SDL.h"

// parameter is UI element ID of clicked button
typedef void (*ButtonHandler)(int);

/**
 * Adds a new button.
 * @param text button text
 * @param bounds the button's bounds
 * @param clickHandler Invoked when the button is clicked
 * @return the button's ID (always >0) if everything worked ok, otherwise 0
 */
int ui_add_button(char *text,SDL_Rect *bounds,ButtonHandler clickHandler);

int ui_run_test();

#endif
