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
    render_free_element(current);
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
    
    int buttonId = uniqueUIElementId++;
    entry->elementId = buttonId;
    
    uiElements = entry;
    
    log_info("ui_add_element: Added element with type %d and ID %d",entry->type,entry->elementId);
    
    pthread_mutex_unlock(&ui_mutex);    
    
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
          render_free_element( current );
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
    ui_element *element = render_allocate_element( UI_BUTTON );
    if ( ! element ) {
      log_error("ui_add_button(): Failed to allocate memory");      
      return 0;  
    }
    
    button_entry *entry = calloc(1,sizeof(button_entry));
    if ( entry == NULL ) 
    {
      render_free_element( element );
      log_error("ui_add_button(): Failed to allocate memory");
      return 0;
    }
    element->button = entry;
    
    entry->clickHandler = clickHandler;
    entry->pressed=0;       
    entry->cornerRadius = 3;
    element->bounds = *bounds;
    entry->text = strdup(text);
    
    int result = render_draw(element);
    if ( result ) 
    {
      return ui_add_element(element);
    } else {
      log_error("ui_add_button(): Failed to render button");      
    }
    render_free_element(element);
    return 0;
}

int ui_add_image_button(char *image,SDL_Rect *bounds,ButtonHandler clickHandler) 
{
    ui_element *element = render_allocate_element( UI_BUTTON );
    if ( ! element ) {
      log_error("ui_add_image_button(): Failed to allocate image button");
      return 0;
    }
    button_entry *entry = calloc(1,sizeof(button_entry));
    if ( entry == NULL ) 
    {
      render_free_element(element);
      log_error("ui_add_image_button(): Failed to allocate image button");
      return 0;
    }
    element->button = entry;
    
    entry->clickHandler = clickHandler;
    entry->pressed=0;       
    entry->cornerRadius = 3;
    element->bounds = *bounds;
    entry->image = render_load_image(image);
    
    if ( entry->image && render_draw(element) ) 
    {
      return ui_add_element(element);        
    }
    render_free_element(element);
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
  ui_element *element = render_allocate_element( UI_LISTVIEW );
  if ( ! element ) {
    log_error("ui_add_listview(): Failed to allocate element");
    return 0;        
  }
  
  listview_entry *entry = calloc(1,sizeof(listview_entry));
  if ( ! entry  ) {
    render_free_element(element);
    log_error("ui_add_listview(): Failed to allocate entry");
    return 0;  
  }
  element->listview = entry;
  
  entry->labelProvider = labelProvider;
  entry->itemCountProvider = itemCountProvider;
  entry->clickCallback = clickCallback;
  
  entry->yStartOffset = 15;
  entry->visibleItemCount=5;
  
  element->bounds.x = bounds->x;
  element->bounds.y = bounds->y;
  element->bounds.w = bounds->w;
  element->bounds.h = entry->visibleItemCount * LISTVIEW_ITEM_HEIGHT;  
  
  if ( render_draw(element) ) 
  {
    return ui_add_element(element);
  }
  render_free_element(element);  
  return 0;
}

// ======================================== END listview ==================

static ui_element * ui_handle_touch_event_button(ui_element *element,TouchEvent *event) 
{
  button_entry *button = element == NULL ? NULL : element->button;
  button_entry *focusedButton = focusedElement == NULL ? NULL : focusedElement->button;
  
  if ( event->type == TOUCH_START ) 
  {
    log_info("ui_handle_touch_event_button(): Clicked button");
    if ( focusedButton ) 
    {
      focusedButton->pressed = 0;
      log_info("rendering button as NOT pressed (TOUCH_START)\n");          
      render_draw(focusedElement);
    }
    
    if ( button ) 
    {
      button->pressed = 1;
      log_info("rendering button as pressed\n");           
      render_draw(element);               
      focusedElement = element;    
    } else {
      focusedElement = NULL;
    }
  } 
  else if ( event->type == TOUCH_STOP ) 
  {
    if ( focusedButton && element && element == focusedElement) 
    {
      focusedButton->pressed = 0;
      log_info("rendering button as NOT pressed (TOUCH_STOP)\n");            
      render_draw(focusedElement);           
      
      log_debug("Detected click on '%s'\n",focusedButton->text);
      focusedButton->clickHandler(focusedElement->elementId);           
    }
    focusedElement = NULL;    
  } 
  else if ( event->type == TOUCH_CONTINUE ) 
  {
    if ( focusedButton ) 
    {    
      if ( button == NULL || element != focusedElement ) 
      {
        focusedButton->pressed = 0;
        log_info("rendering button as NOT pressed (TOUCH_CONTINUE)\n");            
        render_draw(focusedElement);           
        focusedElement = NULL;       
      } 
    } 
    else 
    {
      if ( button && ! button->pressed ) 
      {
        button->pressed = 1;
        log_info("rendering button as pressed (TOUCH_CONTINUE)\n");            
        render_draw(element);           
        focusedElement = element;            
      }      
    }
  }  
  
  pthread_mutex_unlock(&ui_mutex);  
  return focusedElement;
}

static ui_element * ui_handle_touch_event_listview(ui_element *element, TouchEvent *event) 
{
  listview_entry *listview = NULL;
  int listViewId = -1;
  if ( focusedElement ) {
    listview = focusedElement->listview;
    listViewId = focusedElement->elementId;
  } else if ( element ) {
    listview = element->listview;
    listViewId = element->elementId;    
  }
  log_info("ui_handle_touch_event_listview(): listview %d clicked",listViewId);
  
  int clickedItemIdx = 0;
  ListViewClickCallback callbackToInvoke = NULL;
  
  if ( event->type == TOUCH_START ) 
  {
    log_info("ui_handle_touch_event_listview(): TOUCH_START listview %d",listViewId);    
    focusedElement = element;
    listViewMaxYDelta = 0;
    listViewTouchStartY = event->y;
  } 
  else if ( event->type == TOUCH_STOP ) 
  {
    if ( focusedElement != NULL && listViewMaxYDelta <= LISTVIEW_CLICK_MAXDELTA_Y ) 
    {
      log_info("ui_handle_touch_event_listview(): TOUCH_STOP listview %d",listViewId);         
      // calculate item index
      int realY = (listViewTouchStartY - focusedElement->bounds.y) + listview->yStartOffset;
      clickedItemIdx = ( realY / LISTVIEW_ITEM_HEIGHT ); 
      callbackToInvoke = listview->clickCallback;
    }
    focusedElement = NULL;
    listViewMaxYDelta = 0;
    listViewTouchStartY = event->y;    
  } 
  else if ( event->type == TOUCH_CONTINUE ) 
  {
    log_info("ui_handle_touch_event_listview(): TOUCH_CONTINUE listview %d",listViewId);         
    int delta = listViewTouchStartY - event->y;
    if ( abs(delta) > listViewMaxYDelta ) {
        listViewMaxYDelta = abs(delta);
    }
    int newOffset = listview->yStartOffset + delta/2;
    
    int items = listview->itemCountProvider(focusedElement->elementId);
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
    render_draw(focusedElement);
  }
  
  pthread_mutex_unlock(&ui_mutex);
  
  if ( callbackToInvoke != NULL ) 
  {
    callbackToInvoke(listViewId,clickedItemIdx);
  }
  return focusedElement;  
}

void ui_handle_touch_event(TouchEvent *event) 
{
  log_info("ui_handle_touch_event(): Called");
  
  pthread_mutex_lock(&ui_mutex);
  
  ui_element *element = ui_find_element_nolock(event->x,event->y);
  
  if ( element || focusedElement ) 
  {
    UIElementType type = focusedElement ? focusedElement->type : element->type;
    switch(type) 
    {
      case UI_BUTTON:
        // Function UNLOCKS mutex          
        focusedElement = ui_handle_touch_event_button(element, event);
        return;
      case UI_LISTVIEW:
        // Function UNLOCKS mutex
        focusedElement = ui_handle_touch_event_listview(element, event);
        return;        
      default:
        log_error("ui_handle_touch_event(): Unhandled element type %d",type);
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
  ui_free_all();  
  render_close_render();    
}