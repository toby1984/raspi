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

static int uniqueUIElementId = 1; // must start with 1 as 0 is used to signal an error

static pthread_mutex_t button_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void ui_free_button_entry(button_entry *entry) {
    free(entry->desc.text);
    free(entry);   
}

void ui_free_all_button_entries(void) 
{
  pthread_mutex_lock(&button_list_mutex);
  
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
  pthread_mutex_unlock(&button_list_mutex);   
}

int ui_add_button_entry(button_entry *entry) {
    
    pthread_mutex_lock(&button_list_mutex);
    
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
    
    pthread_mutex_unlock(&button_list_mutex);    
    log_info("add_button: Added button with ID %d\n",buttonId);
    return buttonId;
}

int ui_remove_button_entry(button_entry *entry) 
{
    int removed = 0;
    
    pthread_mutex_lock(&button_list_mutex);
    
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
    
    pthread_mutex_unlock(&button_list_mutex);
    if ( ! removed ) {
      log_error("Failed to remove button entry %ld\n",entry);    
    }
    return removed;
}

button_entry *ui_find_button(int x,int y) 
{
    button_entry *result = NULL;
    SDL_Rect *bounds;
    
    pthread_mutex_lock(&button_list_mutex);
    
    button_entry *current = ui_button_first;
    while ( current != NULL ) {
        bounds = &current->desc.bounds;
        if ( x >= bounds->x && y >= bounds->y && x <= (bounds->x + bounds->w) && y <= (bounds->y + bounds->h) ) {
          result = current;
          break;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&button_list_mutex);          
    return result;
}

void assign_color(SDL_Color *color,Uint8 r,Uint8 g, Uint8 b) {
   color->r=r;
   color->g=g;
   color->b=b;
}

int ui_add_button(char *text,SDL_Rect *bounds,ButtonHandler clickHandler) 
{
    button_entry *entry = calloc(1,sizeof(button_entry));
    if ( entry == NULL ) {
        return 0;
    }
    entry->clickHandler = clickHandler;
    entry->desc.pressed=0;
    entry->desc.bounds = *bounds;
    assign_color(&entry->desc.borderColor,255,255,255);
    assign_color(&entry->desc.backgroundColor,128,128,128);
    assign_color(&entry->desc.textColor,255,255,255);
    assign_color(&entry->desc.clickedColor,255,255,255);
    entry->desc.text = strdup(text);
    
    int result = render_draw_button(&entry->desc);
    if ( result ) {
      return ui_add_button_entry(entry);
    }
    return result;
}

void ui_handle_touch_event(TouchEvent *event) 
{
  log_info("ui_handle_touch invoked\n");
  
  if ( event->type == TOUCH_START ) {
      button_entry *pressed = ui_find_button(event->x,event->y);
      log_info("Clicked button: ",pressed);      
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
      button_entry *released = ui_find_button(event->x,event->y);
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
    button_entry *current = ui_find_button(event->x,event->y);    
    if ( pressed_button != NULL && pressed_button != current ) 
    {    
        pressed_button->desc.pressed = 0;
        log_info("rendering button as NOT pressed (TOUCH_CONTINUE)\n");            
        render_draw_button(&pressed_button->desc);           
        pressed_button = NULL;       
    }
  }
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
}