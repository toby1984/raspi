#ifndef MYLIB_H
#define MYLIB_H

#include "ui.h"

int mylib_add_button(char *text,int x,int y,int width,int height,ButtonHandler clickHandler);

int mylib_add_image_button(char *imagePath,int x,int y,int width,int height,ButtonHandler clickHandler);

/**
 * Adds a new button.
 * @param bounds the button's bounds
 * @param labelProvider callback that gets invoked to retrieve the label for a given item
 * @param itemCountProvider callback that gets invoked to determine the number of items that are available
 * @param clickCallback callback that gets invoked when the user clicks on an item
 * 
 * @return the listview's ID (always >0) if everything worked ok, otherwise 0
 */
int mylib_add_listview(SDL_Rect *bounds,ListViewLabelProvider labelProvider, ListViewItemCountProvider itemCountProvider, ListViewClickCallback clickCallback);

int mylib_init(void);

void mylib_close(void);

#endif
