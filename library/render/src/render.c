

#include "render.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_getenv.h"
#include <pthread.h>

// #define USE_FB
SDL_Surface* scrMain = NULL;
TTF_Font* font;
int initFlags = 0;
viewport_desc viewportInfo;

// rendering thread init stuff
volatile pthread_cond_t init_condition;
volatile pthread_t renderingThreadId;
volatile pthread_cond_t init_condition = PTHREAD_COND_INITIALIZER;
volatile pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int initResult = 0;

// mailbox

pthread_mutex_t mbox_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct mbox_entry 
{
  mbox_entry *next;
  mbox_entry *prev;
  RenderCallback *func;
  pthread_cond_t *finished_condition;
  pthread_mutex_t *finished_mutex;  
  void *data;
} mbox_entry;

mbox_entry *mbox_first = NULL;
mbox_entry *mbox_last = NULL;

// wait for condition
#define wait_condition(mutex,condition) \
      pthread_mutex_lock(mutex); \
      pthread_cond_wait(condition, mutex); \
      pthread_mutex_unlock(mutex); 
      
// signal condition
#define signal_condition(mutex,condition) \
    pthread_mutex_lock(mutex); \
    pthread_cond_signal(condition); \
    pthread_mutex_unlock(mutext);  
    
#define init_condition(mutex,condition) \
      pthread_mutex_init(mutext, NULL); \
      pthread_cond_init(condition,NULL);    

int is_on_rendering_thread() {
  return renderingThreadId == pthread_self();
}

void assert_rendering_thread() {
  if ( ! is_on_rendering_thread() ) {
    fprintf("Not on rendering thread!\n");  
  }
}

/**
 * Execute callback on rendering thread.
 * 
 * @param callback Callback to invoke from rendering thread.
 * 
 * @return 
 */
int exec_on_thread(RenderCallback *callback,void *data,int awaitCompletion) 
{
    if ( is_on_rendering_thread() ) {
      fprintf("Warning - exec_on_thread() called while on rendering thread\n");        
      callback(data);
      return 0;
    }
    
    mbox_entry *newEntry = calloc(1,sizeof(mbox_entry);
    if ( newEntry == NULL ) {
      return 0;  
    }
    newEntry->data = data;
  
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
    
    pthread_cond_t finished_condition;
    pthread_mutex_t finished_mutex;     
    
    if ( awaitCompletion ) {
      
      init_condition(&finished_mutex,&finished_condition);
      
      newEntry->finished_condition = &finished_condition;
      newEntry->finished_mutex = &finished_mutex;
    }
    
    pthread_mutex_unlock(&mbox_mutex);   
    
    if ( awaitCompletion ) {
      fprintf(stdout,"Awaiting callback completion ...\n");
      
      wait_condition(&finished_mutex,&finished_condition);
      
      fprintf(stdout,"Callback completed.\n");      
    }
    return 1;
}

mbox_entry *poll() 
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
    }
  }  
  pthread_mutex_unlock(&mbox_mutex); 
  
  return entry;
}


int is_initialized() {
  return initFlags == RENDER_FLAG_SDL_INIT | RENDER_FLAG_TTF_INIT | RENDER_FLAG_TTF_FONT_LOADED;
}

int get_viewport_desc(viewport_desc *port) {
  if ( is_initialized ) 
  {
    port->width = viewportInfo.width;
    port->height = viewportInfo.height;
    port->bitsPerPixel = viewportInfo.bitsPerPixel;
    return 1;  
  }
  return 0;  
}

void close_render_internal(void *dummy) 
{
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
}

typedef struct render_text_args {
  const char *text;
  int x;
  int y;
  SDL_Color color;
} render_text_args;

void free(render_text_args *args) {
  free(args->text);
  free(args);
}

void render_text_internal(render_text_args *args) 
{
  SDL_Surface* textSurface = TTF_RenderText_Solid(font, args->text, args->color);
  SDL_Rect srcRect = {0,0,textSurface->w,textSurface->h};
  SDL_Rect dstRect = {args->x,args->y,srcRect.w,srcRect.h};
  SDL_BlitSurface(textSurface, &srcRect, scrMain, &dstRect );  
  
  SDL_FreeSurface(textSurface);
  
  free(args);
}  

void render_text(const char *text,int x,int y,SDL_Color color) 
{
  render_text_args *args = calloc(1,sizeof(render_text_args));
  
  args->text=strdup(text);
  args->x=x;
  args->y=y;
  memcpy(&args->color,&color,sizeof(SDL_Color));
  
  exec_on_thread( &render_text_internal, args );
}

int init_render_internal(void *dummy) 
{
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
    fprintf(stderr,"ERROR in SDL_Init(): %s\n",SDL_GetError());
    close_render();
    return 0;
  }
  initFlags |= RENDER_FLAG_SDL_INIT;

  // Fetch the best video mode
  // - Note that the Raspberry Pi generally defaults
  //   to a 16bits/pixel framebuffer
  const SDL_VideoInfo* vInfo = SDL_GetVideoInfo();
  if (!vInfo) {
    fprintf(stderr,"ERROR in SDL_GetVideoInfo(): %s\n",SDL_GetError());
    close_render();
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
    fprintf(stderr,"ERROR in SDL_SetVideoMode(): %s\n",SDL_GetError());
    close_render();
    return 0;
  }

  // --------------------------------------
  // Setup TTF
  // --------------------------------------

  if (TTF_Init() < 0) {
    fprintf(stderr,"ERROR in TTF_Init(): %s\n",TTF_GetError());
    close_render();
    return 0;
  }
  
  initFlags |= RENDER_FLAG_TTF_INIT;  

  font = TTF_OpenFont(FONT_PATH, 32);
  if ( ! font ) {
    fprintf(stderr,"Failed to load TTF font %s. %s\n",FONT_PATH,TTF_GetError());
    close_render();
    return 0;
  }
  initFlags |= RENDER_FLAG_TTF_FONT_LOADED;
  
  return 1;
}

void init_render_on_thread() 
{
    fprintf(stdout,"Rendering thread starting...\n");  
    
    pthread_mutex_lock(&init_mutex);
    
    initResult = init_render_internal();
    
    pthread_cond_signal(&init_condition);
    pthread_mutex_unlock(&init_mutex);  
    
    fprintf(stdout,"Initializing rendering on separate thread DONE...\n");      
    
    int terminate = 0;
    while ( ! terminate ) 
    {
      mbox_entry *entry = NULL;
      while ( ! terminate && ( entry = poll() ) ) 
      {
          if ( entry == &close_render_internal) {
            fprintf(stdout,"Rendering thread shutting down...\n");              
            terminate = true;  
          }
          entry->func(entry->data);
          
          if ( entry->finished_mutex ) 
          {
            signal_condition(entry->finished_mutex,entry->finished_condition);            
          }
          
          free(entry);
      }      
      SDL_Flip(scrMain);       
      SDL_Delay(200);        
    }
    fprintf(stdout,"Rendering thread terminated.\n");      
}

int init_render() 
{
  initResult = 0;
  
  int err = pthread_create(&renderingThreadId, NULL, &init_render_on_thread, NULL); 
  if ( err != 0 ) {
    fprintf(stderr,"ERROR - failed to spawn rendering thread\n");
    return 0;  
  }
  
  fprintf(stdout,"Waiting for init_render()...\n");
  pthread_mutex_lock(&init_mutex);
  pthread_cond_wait(&init_condition, &init_mutex);
  pthread_mutex_unlock(&init_mutex);  
  fprintf(stdout,"init_render() returned %d\n",initResult);
  
  return initResult;
}

void close_render() 
{
  exec_on_thread( &close_render_internal );
  initFlags = 0;
}