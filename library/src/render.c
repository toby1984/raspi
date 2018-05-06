#include "render.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_gfx.h"
#include "SDL/SDL_getenv.h"
#include <pthread.h>
#include "log.h"
#include "input.h"
#include <stdarg.h>

SDL_Surface* scrMain = NULL;

TTF_Font* font = NULL;

int initFlags = 0;

viewport_desc viewportInfo = {0};

volatile pthread_t renderingThreadId;

// rendering thread init stuff
pthread_cond_t init_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;

volatile int initResult = 0;

volatile const char *lastRenderError;

typedef struct render_text_args {
  const char *text;
  int x;
  int y;
  SDL_Color color;
} render_text_args;

// mailbox

pthread_mutex_t mbox_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MBOX_FLAG_FREED_BY_CREATOR 1<<0

typedef struct mbox_entry 
{
  struct mbox_entry *next;
  struct mbox_entry *prev;
  RenderCallback func;
  int flags;
  pthread_cond_t *finished_condition;
  pthread_mutex_t *finished_mutex;  
  void *data;
  int result;
} mbox_entry;

mbox_entry *mbox_first = NULL;
mbox_entry *mbox_last = NULL;

// wait for condition
#define render_wait_condition(mutex,condition) \
      pthread_mutex_lock(mutex); \
      pthread_cond_wait(condition, mutex); \
      pthread_mutex_unlock(mutex); 
      
// signal condition
#define render_signal_condition(mutex,condition) \
    pthread_mutex_lock(mutex); \
    pthread_cond_signal(condition); \
    pthread_mutex_unlock(mutex);  
    
#define render_init_condition(mutex,condition) \
      pthread_mutex_init(mutex, NULL); \
      pthread_cond_init(condition,NULL);    

void render_error(const char* msg,...) 
{
  char buffer[1024];  
  va_list ap;
  
  buffer[0]=0;
  
  va_start(ap, msg); //Requires the last fixed parameter (to get the address)
  vsnprintf(&buffer[0], sizeof(buffer), msg, ap);
  va_end(ap);
  
  log_error("%s",&buffer);
    
  lastRenderError = msg;
}   

void render_success() {
  lastRenderError = NULL;    
}

int render_is_on_rendering_thread() {
  return pthread_equal(renderingThreadId,pthread_self());
}

void render_assert_rendering_thread() {
  if ( ! render_is_on_rendering_thread() ) {
      render_error("Not on rendering thread!");  
  }
}

/**
 * Execute callback on rendering thread.
 * 
 * @param callback Callback to invoke from rendering thread.
 * @param data data passed to callback
 * @param awaitCompletion whether to block until the callback has been invoked
 * 
 * @return callback result
 */
int render_exec_on_thread(RenderCallback callback,void *data,int awaitCompletion) 
{
    int result = 0;
    pthread_cond_t finished_condition;
    pthread_mutex_t finished_mutex;     
    
    log_debug("exec_on_thread() called");   
    if ( render_is_on_rendering_thread() ) {
      render_error("Warning - exec_on_thread() called while on rendering thread");        
      callback(data);
      return 1;
    }
    
    // allocate memory for mail box entry
    mbox_entry *newEntry = calloc(1,sizeof(mbox_entry));
    if ( newEntry == NULL ) {
      return 0;  
    }
    if ( awaitCompletion ) 
    {
      newEntry->flags |= MBOX_FLAG_FREED_BY_CREATOR;   
      
      render_init_condition(&finished_mutex,&finished_condition);
      
      newEntry->finished_condition = &finished_condition;
      newEntry->finished_mutex = &finished_mutex;      
    }
    
    newEntry->func = callback;
    newEntry->data = data;

    // insert 
    pthread_mutex_lock(&mbox_mutex);
    
    if ( mbox_first == NULL ) 
    {
      mbox_first = newEntry;
      mbox_last = newEntry;
    }
    else 
    {
      mbox_last->next = newEntry;
      newEntry->prev=mbox_last;
      mbox_last = newEntry;
    }
    
    pthread_mutex_unlock(&mbox_mutex);   
    
    if ( awaitCompletion ) 
    {
      log_debug("Awaiting callback completion ...\n");
      
      render_wait_condition(&finished_mutex,&finished_condition);
      
      log_debug("Callback completed.\n");      
      result = newEntry->result;
      free( newEntry );
    }
    return result;
}

/**
 * Poll mailbox.
 * 
 * @return mbox entry or NULL
 */
mbox_entry *render_poll_mbox() 
{
  pthread_mutex_lock(&mbox_mutex);
  
  mbox_entry *entry = mbox_last;
  if ( entry != NULL ) 
  {
    if ( entry->prev == NULL ) {
      mbox_first = NULL;
      mbox_last = NULL;
    } else {
      mbox_last = entry->prev;
      mbox_last->next=NULL;
    }
  }  
  pthread_mutex_unlock(&mbox_mutex); 
  
  return entry;
}

/**
 * Returns whether the rendering system was initialized.
 * @return 
 */
int render_is_initialized() 
{
  if ( initFlags == (RENDER_FLAG_SDL_INIT | RENDER_FLAG_TTF_INIT | RENDER_FLAG_TTF_FONT_LOADED) ) {
    return 1;
  }
  return 0;
}

/**
 * Fills out a viewport description
 * @return 
 */
int render_get_viewport_desc_internal(viewport_desc *port) 
{
  log_debug("get_viewport_desc_internal() called");   
  port->width = viewportInfo.width;
  port->height = viewportInfo.height;
  port->bitsPerPixel = viewportInfo.bitsPerPixel;
  render_success();  
  return 1;  
}

/**
 * Fills out a viewport description
 * @return 
 */
int render_get_viewport_desc(viewport_desc *port) 
{
  log_debug("get_viewport_desc() called");  
  return render_exec_on_thread(&render_get_viewport_desc_internal,port,1); 
}

int render_close_render_internal(void *dummy) 
{
  log_debug("close_render_internal() called");
  // close font
  if ( initFlags & RENDER_FLAG_TTF_FONT_LOADED) {
    TTF_CloseFont(font);
  }

  // Close down TTF
  if ( initFlags & RENDER_FLAG_TTF_INIT) {
    TTF_Quit();
  }

  // Close down SDL
  if ( initFlags & RENDER_FLAG_SDL_INIT ) {
    SDL_Quit();
  }
  initFlags = 0;
  render_success();
  return 1;
}

void render_close_render() {
  render_exec_on_thread(&render_close_render_internal,NULL,1); 
}

void render_free_render_text_args(render_text_args *args) {
  free(args->text);
  free(args);
}

int render_render_text_internal(render_text_args *args) 
{
  SDL_Surface* textSurface = TTF_RenderText_Solid(font, args->text, args->color);
  SDL_Rect srcRect = {0,0,textSurface->w,textSurface->h};
  SDL_Rect dstRect = {args->x,args->y,srcRect.w,srcRect.h};
  SDL_BlitSurface(textSurface, &srcRect, scrMain, &dstRect );  
  
  SDL_FreeSurface(textSurface);
  
  render_free_render_text_args(args);
  
  render_success();  
  return 1;
}  

void render_render_text(const char *text,int x,int y,SDL_Color color) 
{
  render_text_args *args = calloc(1,sizeof(render_text_args));
  
  args->text=strdup(text);
  args->x=x;
  args->y=y;
  memcpy(&args->color,&color,sizeof(SDL_Color));
  
  render_exec_on_thread( &render_render_text_internal, args,0);
}

int render_init_render_internal() 
{
  log_info("init_render_internal() called \n");
  // --------------------------------------
  // Initialization
  // --------------------------------------

#ifdef USE_FB
 // Update the environment variables for SDL to
 // work correctly with the external display on
 // LINUX frame buffer 1 (fb1).
 putenv((char*)"FRAMEBUFFER=/dev/fb1");
 putenv((char*)"SDL_FBDEV=/dev/fb1");
#endif

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    render_error("SDL_Init() failed: %s",SDL_GetError());
    render_close_render();
    return 0;
  }
  initFlags |= RENDER_FLAG_SDL_INIT;

  // Fetch the best video mode
  // - Note that the Raspberry Pi generally defaults
  //   to a 16bits/pixel framebuffer
  const SDL_VideoInfo* vInfo = SDL_GetVideoInfo();
  if (!vInfo) {
    render_error("SDL_GetVideoInfo() failed: %s",SDL_GetError());
    render_close_render();
    return 0;
  }
  
  viewportInfo.width = 320; // vInfo->current_w;
  viewportInfo.height = 240; // vInfo->current_h;
  viewportInfo.bitsPerPixel = 16; // vInfo->vfmt->BitsPerPixel;

  // Configure the video mode
  // - SDL_SWSURFACE appears to be most robust mode
  int     nFlags = SDL_SWSURFACE;
  scrMain = SDL_SetVideoMode(viewportInfo.width,viewportInfo.height,viewportInfo.bitsPerPixel,nFlags);
  if (scrMain == 0) {
    render_error("SDL_SetVideoMode() failed: %s",SDL_GetError());
    render_close_render();
    return 0;
  }

  // --------------------------------------
  // Setup TTF
  // --------------------------------------

  if (TTF_Init() < 0) {
    render_error("TTF_Init() failed: %",TTF_GetError());
    render_close_render();
    return 0;
  }
  
  initFlags |= RENDER_FLAG_TTF_INIT;  

  font = TTF_OpenFont(FONT_PATH, 32);
  if ( ! font ) {
    render_error("Failed to load TTF font %s. %s",FONT_PATH,TTF_GetError());
    render_close_render();
    return 0;
  }
  initFlags |= RENDER_FLAG_TTF_FONT_LOADED;
  
  render_success();
  
  return 1;
}

void *render_main_event_loop(void* data) 
{
    TouchEvent touchEvent;
    
    initResult = render_init_render_internal();

    render_signal_condition(&init_mutex,&init_condition);
    
    if ( ! initResult ) {
      render_error("init_render_internal() failed");        
      return 0;
    }
    
    log_info("Initializing rendering on separate thread DONE...");      
    
    int terminate = 0;
    while ( ! terminate ) 
    {
      mbox_entry *entry = NULL;
      while ( ! terminate && ( entry = render_poll_mbox() ) ) 
      {
          if ( entry->func == &render_close_render_internal) {
            log_info("Rendering thread shutting down...");              
            terminate = 1;  
          }
         entry->result = entry->func(entry->data);
          
          if ( (entry->flags & MBOX_FLAG_FREED_BY_CREATOR) != 0 ) { // code awaiting completion will free the entry
            render_signal_condition(entry->finished_mutex,entry->finished_condition);  
          } else {
            free(entry);
          }
      }           
      if ( ! terminate ) {
          
        if ( input_poll_touch(&touchEvent) ) {
            log_debug("Touch event at (%d,%d)",touchEvent.x,touchEvent.y);
        }
        SDL_Flip(scrMain);       
        SDL_Delay(20);        
      }
    }
    log_info("Rendering thread terminated.");      
    return 0;
}

int render_init_render() 
{
  log_debug("init_render() called.");  
  if ( render_is_initialized() ) {
    return 1;
  }
  initResult = 0;
  
  int err = pthread_create(&renderingThreadId, NULL, &render_main_event_loop, NULL); 
  if ( err != 0 ) {
    render_error("ERROR - failed to spawn rendering thread");
    return 0;  
  }
  
  log_info("Waiting for init_render_internal()...");
  
  render_wait_condition(&init_mutex,&init_condition);
  
  log_info("init_render() returned %d",initResult);
  return initResult;
}

int render_has_error() {
    return lastRenderError != NULL;
}

volatile const char* render_get_error() {
  return lastRenderError;    
    
}

/**
 * Draws a rectangle using the given color.
 */
int render_draw_button_internal(button_desc *button) {
 /*
  *boxRGBA (SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
 	Draw box (filled rectangle) with blending. 
 	
roundedBoxRGBA (SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
 	Draw rounded-corner box (filled rectangle) with blending.  	
 	
roundedRectangleColor (SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 rad, Uint32 color)
 	Draw rounded-corner rectangle with blending.  	
  */   
 
  Sint16 x1 = button->bounds.x;
  Sint16 y1 = button->bounds.y;
  Sint16 x2 = button->bounds.x+button->bounds.w;
  Sint16 y2 = button->bounds.y+button->bounds.h;
  roundedBoxRGBA(srcMain,x1,y1,x2,y2,
                 button->backgroundColor.r,
                 button->backgroundColor.g,
                 button->backgroundColor.b,
                 button->backgroundColor.a);

  roundedBoxRGBA(srcMain,x1,y1,x2,y2,
                 button->borderColor.r,
                 button->borderColor.g,
                 button->borderColor.b,
                 button->borderColor.a);  
  
  int textWidth;
  int textHeight;
  
  int result = TTF_SizeText(font, button->text, &textWidth, &textHeight);
  if ( ! result ) {
     render_error("Failed to size text");
    return 0;    
  }
  int textX = button->bounds.x + button->bounds.w/2 - textWidth/2;
  int textY = button->bounds.y + button->bounds.h/2 - textHeight/2;
 
  render_text_args text;
  text.text = button->text;
  text.x = textX;
  text.y = textY;
  text.color = button->textColor; 
  
  render_text_internal(&text);
  return 1;
}

int  render_draw_button(button_desc *button) {
    return render_exec_on_thread(render_draw_button_internal,button,1);
}