#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "dynamicstring.h"
#include "SDL/SDL.h"

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

// void callback(textfield_id,entered string)
typedef void (*TextFieldCallback)(int,const char *);

typedef enum { UI_BUTTON, UI_LISTVIEW, UI_TEXTFIELD } UIElementType;

/*
 * Attributes common to all UI elements.
 */
typedef struct ui_element 
{
  struct ui_element *next;
  UIElementType type;
  int elementId;
  union {
    void *elementData;      
    struct listview_entry *listview;
    struct button_entry *button;
    struct textfield_entry *textfield;
  };
  SDL_Rect bounds;    
  SDL_Color borderColor;
  SDL_Color backgroundColor;
  SDL_Color foregroundColor;  
} ui_element;


/*
 * A list view.
 */
typedef struct listview_entry
{
  ListViewLabelProvider labelProvider;
  ListViewItemCountProvider itemCountProvider;
  ListViewClickCallback clickCallback;
  int visibleItemCount;
  int yStartOffset;  
} listview_entry;

/*
 * A button.
 */

typedef struct button_entry 
{
  ButtonHandler clickHandler;
  SDL_Color clickedColor;    
  int cornerRadius;
  int roundedCorners;
  int fontSize;
  int pressed;
  char *text;
  SDL_Surface *image;
} button_entry;

/*
 * A text field.
 */
typedef struct textfield_entry
{
  dynamic_string *content;
  int fontSize;
  int caretPosition;  
} textfield_entry;

#endif