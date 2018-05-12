#include "dynamicstring.h"
#include <stdlib.h>
#include <string.h>

dynamic_string *dynstring_allocate(int initialCapacity) 
{
  dynamic_string *result = calloc(1,sizeof(dynamic_string));
  if ( result ) 
  {
    result->memory = calloc(1,initialCapacity);
    if ( ! result->memory ) {
    log_error("buffer_allocat(): Failed to allocate %d bytes\n",initialCapacity);  
      free(result);
      return NULL;
    }
    result->capacity = initialCapacity;
  } else {
    log_error("buffer_allocat(): Failed to allocate %d bytes\n",sizeof(dynamic_string));  
  }
  return result;
}

int dynstring_insert_at(dynamic_string *buffer,char c,int position) 
{
  dynamic_string *current = buffer;
  
  if ( buffer->len >= buffer->capacity ) 
  {
        int newCapacity = buffer->capacity*2;
        char *newBuffer = calloc(1,newCapacity);
        if ( ! newBuffer ) 
        {
          log_error("dynstring_write(): Failed to allocate new buffer\n");
          return 0;
        }
        memcpy(newBuffer,current->memory,current->len);
        free( current-> memory );
        current->memory = newBuffer;
        current->capacity = newCapacity;    
  }
  if ( position >= current->len ) 
  {
    current->memory[current->len] = c;  
  } else {
    current->memory[position]=c;
  }
  return 1;  
}


void dynstring_free(dynamic_string *buffer) {
  free(buffer->memory);
  free(buffer);  
}