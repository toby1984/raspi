#include "ui.h"
#include "input.h"
#include "render.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include <stdio.h>
#include "log.h"
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "global.h"

static pthread_mutex_t ui_mutex = PTHREAD_MUTEX_INITIALIZER;

static int uniqueUIElementId = 1; // must start with 1 as 0 is used to signal an error

/*
 * Buttons
 */

typedef struct button_entry {
  struct button_entry *next;
  struct button_entry *prev;
  int buttonId;
  ButtonHandler clickHandler;
  button_desc desc;
} button_entry;

static button_entry *ui_button_first=NULL;
static button_entry *ui_button_last=NULL;

// button that is currently pressed down by the user
// but not released yet
static button_entry *pressed_button = NULL;

/*
 * List views
 */

static listview_entry *listViews = NULL;

static listview_entry *activeListView = NULL;
static int listViewMaxYDelta = 0;
static int listViewTouchStartY = -1;

/**
 * Free button entry.
 */
void ui_free_button_entry(button_entry *entry) {
    if ( entry->desc.image != NULL ) 
    {
      render_free_surface(entry->desc.image);
    }
    free(entry->desc.text);
    free(entry);   
}

void ui_free_all_button_entries(void) 
{
  pthread_mutex_lock(&ui_mutex);
  
  button_entry *current=ui_button_first;
  button_entry *next;
  
  while ( current ) 
  {
      next = current->next;
      ui_free_button_entry(current);
      current = next;
  }
  ui_button_first = NULL;
  ui_button_last = NULL;
  pthread_mutex_unlock(&ui_mutex);   
}

void ui_free_listview_entry(listview_entry *listview) {

  free(listview);  
}

void ui_free_all_listview_entries(void) 
{
  listview_entry *next;
  
  pthread_mutex_lock(&ui_mutex);
  listview_entry *current = listViews;
  while ( current ) 
  {
    next=current->next;
    ui_free_listview_entry(current);
    current = next;
  }
  listViews = NULL;
  activeListView = NULL;
  pthread_mutex_unlock(&ui_mutex);  
}

int ui_add_button_entry(button_entry *entry) {
    
    pthread_mutex_lock(&ui_mutex);
    
    int buttonId = uniqueUIElementId;
    uniqueUIElementId++;
    entry->buttonId = buttonId;
    
    
    if ( ui_button_last == NULL ) {
      ui_button_first = entry;    
      ui_button_last = entry;
    } else {
      ui_button_last->next = entry;
      entry->prev = ui_button_last;
      ui_button_last = entry;
    }
    
    pthread_mutex_unlock(&ui_mutex);    
    log_info("add_button: Added button with ID %d\n",buttonId);
    return buttonId;
}

int ui_remove_button_entry(button_entry *entry) 
{
    int removed = 0;
    
    pthread_mutex_lock(&ui_mutex);
    
    button_entry *current = ui_button_first;
    while ( current != NULL ) {
      if ( current == entry ) {
          if ( ui_button_first == entry ) 
          {
            if ( ui_button_first == ui_button_last ) {
              ui_button_first = NULL;
              ui_button_last = NULL;
            } else {
              ui_button_first = entry -> next;
              entry->next->prev = NULL;
            }            
          } 
          else if ( ui_button_last == entry ) 
          {
            ui_button_last = entry->prev;    
          } 
          else {
            entry->next->prev = entry->prev;
            entry->prev->next = entry->next;
          }
          ui_free_button_entry(entry);
          removed = 1;
          break;
      }
    }
    
    pthread_mutex_unlock(&ui_mutex);
    if ( ! removed ) {
      log_error("Failed to remove button entry %ld\n",entry);    
    }
    return removed;
}

#define CONTAINS(bounds,x,y) (( x >= (bounds)->x && y >= (bounds)->y && x <= ((bounds)->x + (bounds)->w) && y <= ((bounds)->y + (bounds)->h) ) ? 1 : 0)

button_entry *ui_find_button_nolock(int x,int y) 
{
    button_entry *result = NULL;
    SDL_Rect *bounds;
    
    button_entry *current = ui_button_first;
    while ( current != NULL ) {
        bounds = &current->desc.bounds;
        if ( CONTAINS(bounds,x,y) ) {
          result = current;
          break;
        }
        current = current->next;
    }
    return result;
}

button_entry *ui_find_button(int x,int y) 
{
    pthread_mutex_lock(&ui_mutex);
    button_entry *result = ui_find_button_nolock(x,y);
    pthread_mutex_unlock(&ui_mutex);          
    return result;
}

void assign_color(SDL_Color *color,Uint8 r,Uint8 g, Uint8 b) {
   color->r=r;
   color->g=g;
   color->b=b;
}

/**
 * Create a button.
 * 
 * @param text button label
 * @param bounds button bounds
 * @param clickHandler callback invoked when the button is pressed_button
 * 
 * @return 0 on error, otherwise the button ID of the newly created button
 */
int ui_add_button(char *text,SDL_Rect *bounds,ButtonHandler clickHandler) 
{
    button_entry *entry = calloc(1,sizeof(button_entry));
    if ( entry == NULL ) {
        return 0;
    }
    
    entry->clickHandler = clickHandler;
    entry->desc.pressed=0;       
    entry->desc.cornerRadius = 3;
    entry->desc.bounds = *bounds;
    ASSIGN_COLOR(&entry->desc.borderColor,255,255,255);
    ASSIGN_COLOR(&entry->desc.backgroundColor,128,128,128);
    ASSIGN_COLOR(&entry->desc.textColor,255,255,255);
    ASSIGN_COLOR(&entry->desc.clickedColor,255,255,255);
    entry->desc.text = strdup(text);
    
    int result = render_draw_button(&entry->desc);
    if ( result ) {
      return ui_add_button_entry(entry);
    }
    return result;
}

int ui_add_image_button(char *image,SDL_Rect *bounds,ButtonHandler clickHandler) {
    button_entry *entry = calloc(1,sizeof(button_entry));
    if ( entry == NULL ) {
        return 0;
    }
    
    entry->clickHandler = clickHandler;
    entry->desc.pressed=0;       
    entry->desc.cornerRadius = 3;
    entry->desc.bounds = *bounds;
    ASSIGN_COLOR(&entry->desc.borderColor,255,255,255);
    ASSIGN_COLOR(&entry->desc.backgroundColor,128,128,128);
    ASSIGN_COLOR(&entry->desc.textColor,255,255,255);
    ASSIGN_COLOR(&entry->desc.clickedColor,255,255,255);
    entry->desc.image = render_load_image(image);
    
    if ( entry->desc.image == NULL ) {
      free(entry);
      return 0;    
    }
    
    int result = render_draw_button(&entry->desc);
    if ( result ) {
      return ui_add_button_entry(entry);
    }
    return result;    
}

// ================================================

/**
 * Finds a listview by screen coordinates.
 * 
 * @param x
 * @param y
 * @return listview_entry or NULL
 */
listview_entry *ui_find_listview_nolock(int x,int y) 
{
  listview_entry *result = NULL;
  
  listview_entry *current = listViews;
  listview_entry *next;
  
  SDL_Rect bounds;
  
  while ( current ) 
  {
    next = current -> next;
    bounds.x = current ->x;
    bounds.y = current ->y;
    bounds.w = current ->width;
    bounds.h = current ->visibleItemCount* LISTVIEW_ITEM_HEIGHT;
    if ( CONTAINS(&bounds,x,y) ) 
    {
      result = current;
      break;
    }
    current = next;
  }
  
  return result;  
}

listview_entry *ui_find_listview(int x,int y) 
{
  pthread_mutex_lock(&ui_mutex);
  listview_entry *result = ui_find_listview_nolock(x,y);
  pthread_mutex_unlock(&ui_mutex);  
  return result;  
}

/**
 * Adds a new button.
 * @param bounds the button's bounds
 * @param labelProvider callback that gets invoked to retrieve the label for a given item
 * @param itemCountProvider callback that gets invoked to determine the number of items that are available
 * @param clickCallback callback that gets invoked when the user clicks on an item
 * 
 * @return the listview's ID (always >0) if everything worked ok, otherwise 0
 */
int ui_add_listview(SDL_Rect *bounds,ListViewLabelProvider labelProvider, ListViewItemCountProvider itemCountProvider, ListViewClickCallback clickCallback) 
{
    listview_entry *entry = calloc(1,sizeof(listview_entry));
    if ( ! entry  ) {
      log_error("ui_add_listview(): Failed to allocate entry");
      return -1;  
    }
    
    pthread_mutex_lock(&ui_mutex);
    
    entry->listViewId = uniqueUIElementId;
    uniqueUIElementId++;
        
    entry->labelProvider = labelProvider;
    entry->itemCountProvider = itemCountProvider;
    entry->clickCallback = clickCallback;
    
    entry->x = bounds->x;
    entry->y = bounds->y;
    entry->width = bounds->w;
    entry->yStartOffset = 15;
    entry->visibleItemCount=5;
    
    if ( listViews == NULL ) {
      entry->next = NULL;
      listViews = entry;
    } else {
      entry->next = listViews;
      listViews = entry;
    }
    
    int result = render_draw_listview(entry);
    
    pthread_mutex_unlock(&ui_mutex);    
    return result;
}

// ======================================== END listview ==================

void ui_handle_touch_event_button(button_entry *button,TouchEvent *event) 
{
  if ( event->type == TOUCH_START ) {
      button_entry *pressed = button;
      log_info("ui_handle_touch_event_button(): Clicked button");
      if ( pressed != NULL ) 
      {        
        if ( pressed_button ) {
          pressed_button->desc.pressed = 0;
          log_info("rendering button as NOT pressed (TOUCH_START)\n");          
          render_draw_button(&pressed_button->desc);
        }
        pressed->desc.pressed = 1;
        pressed_button = pressed;    
        log_info("rendering button as pressed\n");           
        render_draw_button(&pressed_button->desc);        
      }
  } 
  else if ( event->type == TOUCH_STOP ) 
  {
      button_entry *released = button;
      if ( pressed_button != NULL ) 
      {
        pressed_button->desc.pressed = 0;
        log_info("rendering button as NOT pressed (TOUCH_STOP)\n");            
        render_draw_button(&pressed_button->desc);           
        
        if ( released != NULL && pressed_button == released ) 
        {
          log_debug("Detected click on '%s'\n",pressed_button->desc.text);
          pressed_button->clickHandler(pressed_button->buttonId);   
        }        
        pressed_button = NULL;    
      }
  } 
  else if ( event->type == TOUCH_CONTINUE ) 
  {
    button_entry *current = button;    
    if ( pressed_button != NULL && pressed_button != current ) 
    {    
        pressed_button->desc.pressed = 0;
        log_info("rendering button as NOT pressed (TOUCH_CONTINUE)\n");            
        render_draw_button(&pressed_button->desc);           
        pressed_button = NULL;       
    }
  }  
  
  pthread_mutex_unlock(&ui_mutex);  
}

  /*
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
   
static listview_entry *activeListView = NULL;
static int listViewMaxYDelta = 0;
static int listViewTouchStartY = -1;
*/ 

void ui_handle_touch_event_listview(listview_entry *listview, TouchEvent *event) 
{
  log_info("ui_handle_touch_event_listview(): listview %d clicked",listview->listViewId);
  
  int listViewId = listview->listViewId;
  int clickedItemIdx = 0;
  ListViewClickCallback callbackToInvoke = NULL;
  
  if ( event->type == TOUCH_START ) 
  {
    activeListView = listview;
    listViewMaxYDelta = 0;
    listViewTouchStartY = event->y;
  } 
  else if ( event->type == TOUCH_STOP ) 
  {
    if ( activeListView != NULL && listViewMaxYDelta <= LISTVIEW_CLICK_MAXDELTA_Y ) 
    {
      // calculate item index
      int realY = (listViewTouchStartY - listview->y) + listview->yStartOffset;
      clickedItemIdx = ( realY / LISTVIEW_ITEM_HEIGHT ); 
      callbackToInvoke = listview->clickCallback;
    }
    activeListView = NULL;
    listViewMaxYDelta = 0;
    listViewTouchStartY = event->y;    
  } 
  else if ( event->type == TOUCH_CONTINUE ) 
  {
    int delta = listViewTouchStartY - event->y;
    if ( abs(delta) > listViewMaxYDelta ) {
        listViewMaxYDelta = abs(delta);
    }
    int newOffset = listview->yStartOffset + delta/2;
    
    int items = listview->itemCountProvider(listview->listViewId);
    int maxOffset;
    if ( items <= listview->visibleItemCount ) {
      maxOffset = 0;  
    } else {
      maxOffset = items * LISTVIEW_ITEM_HEIGHT - listview->visibleItemCount * LISTVIEW_ITEM_HEIGHT;
    }
    if ( newOffset < 0 ) {
      newOffset = 0;  
    } else if ( newOffset > maxOffset ) {
      newOffset = maxOffset;
    }
    listview->yStartOffset = newOffset;
    render_draw_listview(listview);
  }
  
  pthread_mutex_unlock(&ui_mutex);
  
  if ( callbackToInvoke != NULL ) 
  {
    callbackToInvoke(listViewId,clickedItemIdx);
  }
}


void ui_handle_touch_event(TouchEvent *event) 
{
  log_info("ui_handle_touch invoked\n");
  
  pthread_mutex_lock(&ui_mutex);
  
  button_entry *button = ui_find_button_nolock(event->x,event->y);
  if ( button != NULL ) 
  {
    // Function UNLOCKS mutex    
    ui_handle_touch_event_button(button,event);
    return;
  } 

  listview_entry *listview = ui_find_listview_nolock(event->x,event->y);
  if ( listview != NULL ) 
  {
    // Function UNLOCKS mutex
    ui_handle_touch_event_listview(listview,event);
    return;
  }
  pthread_mutex_unlock(&ui_mutex);
}

int ui_init(void) 
{
  if ( ! render_init_render() ) {
    return 0;  
  }  
  input_set_input_handler(ui_handle_touch_event);    
  return 1;
}

void ui_close(void) 
{
  render_close_render();   
  ui_free_all_button_entries();
  ui_free_all_listview_entries();  
  
}