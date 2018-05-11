#ifndef UI_H
#define UI_H

#include "SDL/SDL.h"

#define LISTVIEW_CLICK_MAXDELTA_Y 3

// parameter is the button ID of the clicked button
typedef void (*ButtonHandler)(int);

// Returns the label to display for item Y of listview X. Item IDs start with 0.
// char* callback(listview_id,item id)
typedef char* (*ListViewLabelProvider)(int,int);

// returns the number of items this listview should display
// int callback(listview_id)
typedef int (*ListViewItemCountProvider)(int);

// invoked whenever the user selects an item from a
// list view. Item IDs start with zero.
// void callback(listview_id,item id)
typedef void (*ListViewClickCallback)(int,int);

typedef struct listview_entry
{
  struct listview_entry *next;
  int listViewId;
  ListViewLabelProvider labelProvider;
  ListViewItemCountProvider itemCountProvider;
  ListViewClickCallback clickCallback;
  int x;
  int y;
  int width;
  int visibleItemCount;
  int yStartOffset;  
} listview_entry;


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

#endif
