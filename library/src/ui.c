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

// all registered UI elements
static ui_element *uiElements = NULL;

// UI element that currently receives all input events
static ui_element *focusedElement = NULL;

/*
 * Data specific to the focused list view (if any)
 */
static int listViewMaxYDelta = 0;
static int listViewTouchStartY = -1;

ui_element *ui_allocate_element(UIElementType type) 
{
    ui_element *element = calloc(1,sizeof(ui_element));
    if ( ! element ) {
      log_error("ui_allocate_element(): Failed to allocate memory");      
      return NULL;  
    }
    element->type = UI_BUTTON;
    
    ASSIGN_COLOR(&element->borderColor,255,255,255);
    ASSIGN_COLOR(&element->backgroundColor,128,128,128);
    ASSIGN_COLOR(&element->foregroundColor,255,255,255);
    
    return element;
}

/**
 * Frees all memory associated with a button entry.
 * 
 * @param entry entry to free
 */
static void ui_free_button_entry_nolock(button_entry *entry) 
{
    if ( entry->image != NULL ) 
    {
      render_free_surface(entry->image);
    }
    free(entry->text);
    free(entry);   
}

/**
 * Frees all memory associated with a listview entry.
 * @param listview
 */
static void ui_free_listview_entry(listview_entry *listview) {

  free(listview);  
}

/**
 * Frees all memory associated with a UI element.
 * @param element
 */
static void ui_free_element_nolock(ui_element *current) 
{
    switch( current->type ) 
    {
      case UI_BUTTON:
        ui_free_button_entry_nolock( current->button);        
        break;        
      case UI_TEXTFIELD:
        break;        
      case UI_LISTVIEW:
        ui_free_listview_entry_nolock( current->listview );
        break;
      default:
        log_error("ui_free_all(): Don't know how to free type %d",current->type);
    }
    free( element );
}

/**
 * Frees all UI elements.
 */
void ui_free_all() 
{
  pthread_mutex_lock(&ui_mutex);
  ui_element *current = uiElements;
  while ( current ) 
  {
    ui_element *next = current -> next;
    ui_free_element_nolock(current);
    current = next;
  }
  uiElements  = NULL;
  pthread_mutex_unlock(&ui_mutex);
}

/**
 * Registers a UI element.
 * @param type element type
 * @param elementData element-specific data
 * @param element ID or zero on error
 */
int ui_add_element(ui_element *entry) 
{    
    pthread_mutex_lock(&ui_mutex);
    
    entry->next = uiElements;
    entry->elementId = uniqueUIElementId++;
    
    uiElements = entry;
    
    pthread_mutex_unlock(&ui_mutex);    
    
    log_info("ui_add_element: Added element with type %d and ID %d",type,buttonId);
    return buttonId;
}

/**
 * Removes (discards) a UI element.
 * @param entry element to discard.
 */
void ui_remove_element(ui_element *entry) 
{
    int removed = 0;
    
    pthread_mutex_lock(&ui_mutex);
    
    ui_element *previous = NULL;    
    ui_element *current = uiElements;
    while ( current ) 
    {
      if ( current == entry ) 
      {
          previous->next = current->next;
          ui_free_element_nolock( current );
          removed = 1;
          break;
      }
      previous = current;
      current = current->next;
    }
    
    pthread_mutex_unlock(&ui_mutex);
    if ( ! removed ) {
      log_error("Failed to remove entry with type %d and ID %d",entry->type,entry->elementId);    
    }
}

#define CONTAINS(bounds,x,y) (( x >= (bounds)->x && y >= (bounds)->y && x <= ((bounds)->x + (bounds)->w) && y <= ((bounds)->y + (bounds)->h) ) ? 1 : 0)

/**
 * Finds the UI element at the given coordinates while NOT aquiring the global lock.
 * @param x
 * @param y
 * @return UI element or NULL
 */
static ui_element *ui_find_element_nolock(int x,int y) 
{
  ui_element * result = NULL;
  
  ui_element *current = uiElements;
  while ( current ) 
  {
    if ( CONTAINS(&current->bounds,x,y) ) 
    {
      result = current;
      break;
    }
    current = current->next;
  }
  return result;
}

/**
 * Finds the UI element at the given coordinates.
 * 
 * @param x
 * @param y
 * @return UI element or NULL
 */
ui_element *ui_find_element(int x,int y) 
{
  pthread_mutex_lock(&ui_mutex);
  ui_element * result = ui_find_element_nolock(x,y);
  pthread_mutex_unlock(&ui_mutex);
  return result;
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
    ui_element *element = calloc(1,sizeof(ui_element));
    if ( ! element ) {
      log_error("ui_add_button(): Failed to allocate memory");      
      return 0;  
    }
    element->type = UI_BUTTON;
    
    button_entry *entry = calloc(1,sizeof(button_entry));
    if ( entry == NULL ) 
    {
      free( 
      log_error("ui_add_button(): Failed to allocate memory");
      return 0;
    }
    element->button = entry;
    
    entry->clickHandler = clickHandler;
    entry->pressed=0;       
    entry->cornerRadius = 3;
    entry->desc.bounds = *bounds;
    ASSIGN_COLOR(&entry->desc.borderColor,255,255,255);
    ASSIGN_COLOR(&entry->desc.backgroundColor,128,128,128);
    ASSIGN_COLOR(&entry->desc.textColor,255,255,255);
    ASSIGN_COLOR(&entry->desc.clickedColor,255,255,255);
    entry->desc.text = strdup(text);
    
    int result = render_draw_button(&entry->desc);
    if ( result ) 
    {
      ui_element *element = ui_add_element(UIElementType.BUTTON,entry);
      if ( element ) 
      {
        return element->elementId;  
      }
      log_error("ui_add_button(): Failed to add button");         
    } else {
      log_error("ui_add_button(): Failed to render button");      
    }
    ui_free_button_entry_nolock(entry);
    return 0;
}

int ui_add_image_button(char *image,SDL_Rect *bounds,ButtonHandler clickHandler) 
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
    entry->desc.image = render_load_image(image);
    
    if ( entry->desc.image ) 
    {
      if ( render_draw_button(&entry->desc) ) 
      {
        ui_element *element = ui_add_element(UIElementType.BUTTON,entry);        
        if ( element ) {
          return element->elementId;
        }
      }      
    }
    ui_free_button_entry_nolock(entry);
    return 0;    
}

// ================================================

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
      return 0;  
    }
    
    entry->labelProvider = labelProvider;
    entry->itemCountProvider = itemCountProvider;
    entry->clickCallback = clickCallback;
    
    entry->x = bounds->x;
    entry->y = bounds->y;
    entry->width = bounds->w;
    entry->yStartOffset = 15;
    entry->visibleItemCount=5;
    
    if ( render_draw_listview(entry) ) 
    {
      ui_element *element = ui_add_element(UIElementType.LISTVIEW,entry);
      if ( element ) 
      {
        return element->elementId;
      }
    }

    ui_free_listview_entry(entry);  
    return 0;
}

// ======================================== END listview ==================

void ui_handle_touch_event_button(ui_element *element,TouchEvent *event) 
{
  button_entry *button = element == NULL ? NULL : element->button;
  button_entry *focusedButton = focusedElement == NULL ? NULL : focusedElement->button;
  
  if ( event->type == TOUCH_START ) 
  {
      log_info("ui_handle_touch_event_button(): Clicked button");
      if ( focusedButton ) {
        focusedButton->desc.pressed = 0;
        log_info("rendering button as NOT pressed (TOUCH_START)\n");          
        render_draw_button(&focusedButton->desc);
      }
      
      if ( button != NULL ) {
        pressed->desc.pressed = 1;
        focusedElement = element;    
        log_info("rendering button as pressed\n");           
        render_draw_button(&button->desc);        
      }
  } 
  else if ( event->type == TOUCH_STOP ) 
  {
      if ( focusedButton != NULL ) 
      {
        focusedButton->desc.pressed = 0;
        log_info("rendering button as NOT pressed (TOUCH_STOP)\n");            
        render_draw_button(&focusedButton->desc);           
        
        if ( button != NULL && focusedButton == button ) 
        {
          log_debug("Detected click on '%s'\n",focusedButton->desc.text);
          focusedButton->clickHandler(focusedButton->buttonId);   
        }        
        
        focusedElement = NULL;    
      }
  } 
  else if ( event->type == TOUCH_CONTINUE ) 
  {
    if ( focusedButton != NULL ) 
    {    
        if ( button == NULL || button != focusedButton ) 
        {
          focusedButton->desc.pressed = 0;
          log_info("rendering button as NOT pressed (TOUCH_CONTINUE)\n");            
          render_draw_button(&focusedButton->desc);           
          focusedElement = NULL;       
        }
    }
  }  
  
  pthread_mutex_unlock(&ui_mutex);  
}

void ui_handle_touch_event_listview(ui_element *element, TouchEvent *event) 
{
  listview_entry *listview = element == NULL ? NULL : element->listview;
  listview_entry *focusedListView = focusedElement == NULL ? NULL : focusedElement->listview;
  
  log_info("ui_handle_touch_event_listview(): listview %d clicked",listview->listViewId);
  
  int listViewId = listview->listViewId;
  int clickedItemIdx = 0;
  ListViewClickCallback callbackToInvoke = NULL;
  
  if ( event->type == TOUCH_START ) 
  {
    focusedElement = element;
    listViewMaxYDelta = 0;
    listViewTouchStartY = event->y;
  } 
  else if ( event->type == TOUCH_STOP ) 
  {
    if ( focusedListView != NULL && listViewMaxYDelta <= LISTVIEW_CLICK_MAXDELTA_Y ) 
    {
      // calculate item index
      int realY = (listViewTouchStartY - listview->y) + listview->yStartOffset;
      clickedItemIdx = ( realY / LISTVIEW_ITEM_HEIGHT ); 
      callbackToInvoke = listview->clickCallback;
    }
    focusedElement = NULL;
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
  log_info("ui_handle_touch_event(): Called");
  
  pthread_mutex_lock(&ui_mutex);
  
  ui_element *element;
  if ( focusedElement != NULL ) 
  {
    element = focusedElement;
  } 
  else 
  {
    element = ui_find_element_nolock(event->x,event->y);
  }
  
  if ( element ) 
  {
    switch(element->type) 
    {
      case UIElementType.BUTTON:
        // Function UNLOCKS mutex          
        ui_handle_touch_event_button((button_entry*) element->elementData,event);
        return;
      case UIElementType.LISTVIEW:
        // Function UNLOCKS mutex
        ui_handle_touch_event_listview(listview,event);
        return;        
      default:
        log_error("ui_handle_touch_event(): Unhandled element type %d",element->type);
    }
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
  ui_free_all();  
}