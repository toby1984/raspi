#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include "SDL/SDL.h"
#include "ui_types.h"

/**
 * Renders a text field. 
 */
int textfield_render(ui_element *tf,SDL_Surface *surface);

/**
 * Inserts a character at a certain position.
 * 
 */
int textfield_insert_character(ui_element *tf,char c,int position);

/**
 * Deletes a character at a certain position.
 * @param tf textfield
 * @param position position to delete character at. 
 */
int textfield_delete_character(ui_element *tf,int position);

/**
 * Allocate a textfield with the given bounds and initial text.
 * @param bounds
 * @param initialText
 * @return textfield
 */
textfield_entry *textfield_allocate(SDL_Rect *bounds,char *initialText);

/**
 * Free the resources associated with a text field.
 */
void textfield_free(ui_element *tf);
#endif