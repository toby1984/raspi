#ifndef UI_H
#define UI_H

#include "SDL/SDL.h"
#include "ui_types.h"

#define LISTVIEW_CLICK_MAXDELTA_Y 3

/**
 * Adds a new button that displays a text label.
 * @param text button text
 * @param bounds the button's bounds
 * @param clickHandler Invoked when the button is clicked
 * @return the button's ID (always >0) if everything worked ok, otherwise 0
 */
int ui_add_button(char *text,SDL_Rect *bounds,ButtonHandler clickHandler);

/**
 * Adds a new button that displays an image.
 * @param image path to image file
 * @param bounds the button's bounds
 * @param clickHandler Invoked when the button is clicked
 * @return the button's ID (always >0) if everything worked ok, otherwise 0
 */
int ui_add_image_button(char *image,SDL_Rect *bounds,ButtonHandler clickHandler);

/**
 * Adds a new button.
 * @param bounds the button's bounds
 * @param labelProvider callback that gets invoked to retrieve the label for a given item
 * @param itemCountProvider callback that gets invoked to determine the number of items that are available
 * @param clickCallback callback that gets invoked when the user clicks on an item
 * 
 * @return the listview's ID (always >0) if everything worked ok, otherwise 0
 */
int ui_add_listview(SDL_Rect *bounds,ListViewLabelProvider labelProvider, ListViewItemCountProvider itemCountProvider, ListViewClickCallback clickCallback);

int ui_run_test(void);

int ui_init(void);

void ui_close(void);

/**
 * Add textfield
 * 
 * @param bounds textfield boundary
 * @param text initial text
 * @param callback callback to invoke after the user changed the textfield
 * 
 * @return textfield ID
 */
int ui_add_textfield(SDL_Rect bounds,char *text,TextFieldCallback callback);

#endif
