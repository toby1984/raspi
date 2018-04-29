#include "uapi/linux/input-event-codes.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// struct input_event {
// struct timeval time;
// unsigned short type;
// unsigned short code;
// unsigned int value;
// };

enum TouchEventType { CLICK, DRAG };

int touchEventsFd = -1;

int initialized = 0;

typedef struct TouchEvent {
  int x;
  int y;
  int type; // TouchEventType
} TouchEvent;



// int open(const char *pathname, int flags);


int init_input() 
{
  const char *inputDevice;
#ifdef USE_MOUSE  
  inputDevice = MOUSE_INPUT;
#else
  inputDevice = "/dev/input/touchscreen";
#endif
  touchEventsFd = open(inputDevice,0);    
  if ( touchEventsFd == -1 ) {
    fprintf(stderr,"ERROR: Failed to open input device %s\n",inputDevice);
    return 0;  
  }
  return 1;
}

void close_input() 
{
  if ( initialized ) 
  {
    close(touchEventsFd);  
    initialized = 0;
  }
}
  
int fill_event_buffer(TouchEvent *buffer,int bufferSize) {
  
  struct input_event tmpBuffer[bufferSize / sizeof(TouchEvent)];
  
  memset(tmpBuffer, 0, sizeof(tmpBuffer));
  memset(buffer, 0, bufferSize);
  
  int result = 0;
  ssize_t read_count = 0;
  
  do {
    read_count = read(touchEventsFd, tmpBuffer, sizeof(tmpBuffer));
    if (read_count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
      fprintf(stderr,"fill_event_buffer() failed\n");
      return -1;
    } 
    else if (read_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) 
    {
      break;
    }
    else if (read_count > 0) 
    {
      int eventsRead = read_count / sizeof(struct input_event);
      if ( eventsRead*sizeof(struct input_event) != read_count ) {
        fprintf(stderr,"fill_event_buffer() failed, read() call returned fractional events?\n");
        return -1;          
      }
      
      for ( int i = 0 ; i < eventsRead ; i++ ) {
           struct input_event *readPtr = &tmpBuffer[i];
           TouchEvent *writePtr = &buffer[i];
           
           /*
typedef struct TouchEvent {
  int x;
  int y;
  int type; // TouchEventType
} TouchEvent;            
            */
           writePtr->x = readPtr->
      }
    }
  } while (read_count > 0);
  
  return result;    
}


int read_events(TouchEvent *buffer,int bufferSize) 
{
  fd_set read_fds;
  fd_set write_fds;
  fd_set except_fds;
  
  // Select() updates fd_set's, so we need to build fd_set's before each select()call.
  FD_ZERO(read_fds);
  FD_SET(touchEventsFd, read_fds);

  FD_ZERO(write_fds);
  // there is smth to send, set up write_fd for server socket

  FD_ZERO(except_fds);
  FD_SET(touchEventsFd, except_fds);

  struct timeval timeout;    
  timeout.tv_sec = 0;
  timeout.tv_usec = 100*1000; // 100ms
  
  int activity = select(maxfd + 1, &read_fds, &write_fds, &except_fds, &timeout);
  
  switch (activity) {
    case -1:
      fprintf(stderr,"select() failed\n");
      return -1;
    case 0:
      // you should never get here
      fprintf(stderr,"select() returned 0\n");
      return -1;
    default:
      /* All fd_set's should be checked. */
      
      if (FD_ISSET(touchEventsFd, &except_fds)) {
        printf("read() failed\n");
        return -1;
      }
      
      if (FD_ISSET(touchEventsFd, &read_fds)) {
        return fill_event_buffer(buffer,bufferSize);
      }
  }
  return 0;  
}
  
