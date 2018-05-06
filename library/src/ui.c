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
    fprintf(stderr,"add_button: Added button with ID %d\n",buttonId);
    log_info("add_button: Added button with ID %d\n",buttonId);
    return buttonId;
}

void ui_free_button_entry(button_entry *entry) {
    free(entry->desc.text);
    free(entry);   
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

int ui_add_button(char *text,SDL_Rect *bounds,ButtonHandler clickHandler) 
{
    button_entry *entry = calloc(1,sizeof(button_entry));
    if ( entry == NULL ) {
        return 0;
    }
    entry->clickHandler = clickHandler;
    entry->desc.bounds = *bounds;
    entry->desc.borderColor.r = 255;
    entry->desc.borderColor.g = 255;
    entry->desc.borderColor.b = 255;
    entry->desc.backgroundColor.r = 128;
    entry->desc.backgroundColor.g = 128;
    entry->desc.backgroundColor.b = 128;
    entry->desc.textColor.r = 255;
    entry->desc.textColor.g = 255;
    entry->desc.textColor.b = 255;
    entry->desc.text = strdup(text);
    
    int result = render_draw_button(&entry->desc);
    if ( result ) {
      return ui_add_button_entry(entry);
    }
    return result;
}

void clickHandler(int buttonId) {
  log_info("button %d clicked!",buttonId);    
}

int ui_run_test_internal(void* data) 
{
  viewport_desc desc;
  render_get_viewport_desc(&desc);
  
  SDL_Rect        rectTmp;
  Uint32          nColTmp;
  for (Uint16 nPosX=0;nPosX<desc.width;nPosX++) {
          rectTmp.x = nPosX;
          rectTmp.y = desc.height/2;
          rectTmp.w = 1;
          rectTmp.h = desc.height/2;
          nColTmp = SDL_MapRGB(scrMain->format,nPosX%256,0,255-(nPosX%256));
          SDL_FillRect(scrMain,&rectTmp,nColTmp);
  }

  // Draw a green box
  Uint32 nColGreen = SDL_MapRGB(scrMain->format,0,255,0);
  SDL_Rect rectBox = {0,0,desc.width,desc.height/2};
  SDL_FillRect(scrMain,&rectBox,nColGreen);

  SDL_Rect buttonBounds = {50,50,150,20};
  ui_add_button("button1",&buttonBounds,clickHandler);
  
  return 1;
}

void ui_handle_touch_event(TouchEvent *event) 
{
  log_info("ui_handle_touch invoked\n");
  
  if ( event->type == TOUCH_START ) {
      button_entry *pressed = ui_find_button(event->x,event->y);
      if ( pressed != NULL ) 
      {        
        pressed_button = pressed;    
      }
  } else if ( event->type == TOUCH_STOP ) {
      button_entry *released = ui_find_button(event->x,event->y);
      if ( pressed_button != NULL && released != NULL ) {
        if ( pressed_button == released ) 
        {
          log_debug("Detected click on '%s'\n",pressed_button->desc.text);
          pressed_button->clickHandler(pressed_button->buttonId);   
        } 
        pressed_button = NULL;    
      }
  }  
}

int ui_init() {
  input_set_input_handler(ui_handle_touch_event);    
  return 1;
}

int ui_run_test() 
{
  if ( ! render_init_render() ) {
    return 0;  
  }

  if ( ! ui_init() ) {
    return 0;    
  }
  
  log_debug("Now calling run_test_internal()...");
  render_exec_on_thread(&ui_run_test_internal,NULL,0);
  
  log_debug("Now sleeping 6 seconds ...");  
  sleep(6);
  log_debug("Now calling close_render()...");
  
  render_close_render();  
  
  return 1;
}