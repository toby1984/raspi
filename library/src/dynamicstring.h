#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#include "log.h"
#include <string.h>

/*
 * A char buffer that dynamically resizes as
 * you write more bytes to it.
 * @author tobias.gierke@code-sourcery.de
 */

typedef struct dynamic_string
{
  char *memory;
  int len; // string length EXCLUDING the trailing zero-byte
  int capacity;  
} dynamic_string;

/**
 * Allocates a dynamic buffer with a given initial size.
 * @param initalCapacity size in bytes
 * @return buffer or NULL on OOM
 */
dynamic_string *dynstring_allocate(int initialCapacity);

/**
 * Write to dynamic buffer at a certain position, increasing it as necessary.
 * @param buffer buffer to write 
 * @param c character to write
 * @param position to insert the character, positions greater than the string's current length will always append at the end
 * @return 0 on failure, otherwise success
 */
int dynstring_insert_at(dynamic_string *buffer,char c,int position);

/**
 * Deletes a character from a dynamic buffer, possibly shrinking it in the process.
 * @param buffer buffer to write 
 * @param c byte to write
 * @return 0 on failure, otherwise success
 */
int dynstring_delete_at(dynamic_string *buffer,int position);

/**
 * Free a dynamic buffer. 
 * @param buffer buffer to free
 */
void dynstring_free(dynamic_string *buffer);

#endif