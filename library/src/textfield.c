#include "render.h"
#include "ui_types.h"

int textfield_render_internal(ui_element *tf) 
{
  // TODO: Implement me!
  return 0;
}

/**
 * Renders a text field. 
 */
int textfield_render(ui_element *tf) 
{
  // TODO: Implement me!
  return (int) render_exec_on_thread(textfield_render_internal,tf,1);
}

/**
 * Inserts a character at a certain position.
 * 
 */
int textfield_insert_character(ui_element *tf,char c,int position) {
  // TODO: Implement me!
  return 0;  
}

/**
 * Deletes a character at a certain position.
 * @param tf textfield
 * @param position position to delete character at. 
 */
int textfield_delete_character(ui_element *tf,int position) {
  // TODO: Implement me!
  return 0;
}