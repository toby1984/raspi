#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void buttonHandler(int buttonId) {
  printf("Button clicked >> %d <<\n",buttonId);
}

/*
 * Adds a new button.
 * @param bounds the button's bounds
 * @param labelProvider callback that gets invoked to retrieve the label for a given item
 * @param itemCountProvider callback that gets invoked to determine the number of items that are available
 * @param clickCallback callback that gets invoked when the user clicks on an item
 * 
 * @return the listview's ID (always >0) if everything worked ok, otherwise 0
int mylib_add_listview(SDL_Rect *bounds,ListViewLabelProvider labelProvider, ListViewItemCountProvider itemCountProvider, ListViewClickCallback clickCallback); 
 */

char *getLabel(int listViewId,int itemId) 
{
  switch(itemId) {
    case 0: return "item #0";    
    case 1: return "item #1";    
    case 2: return "item #2";    
    case 3: return "item #3";    
    case 4: return "item #4";    
    case 5: return "item #5";    
    case 6: return "item #6";    
    default:
      printf(">>>>> getLabel() called with %d\n",itemId);
      return "error";
  }
}

int getItemCount(int listViewId) {
  return 7;  
}

void itemClicked(int listViewId,int itemId) {
  printf("Item %d clicked\n",itemId);
}

int main(int argc, char* args[])
{
  if ( mylib_init() ) 
  {
    // int elementId = mylib_add_image_button("/home/tobi/qtcreator/raspi/raspi/test.png",50,50,150,20,buttonHandler);    
//     int elementId = mylib_add_button("test",50,50,150,20,buttonHandler);
//     printf("Registered button %d\n",elementId);
    
    SDL_Rect bounds = {10,10,150,150};
    int elementId = mylib_add_listview(&bounds, getLabel, getItemCount, itemClicked);
    if ( elementId > 0 ) {
      sleep(10);
    }
    mylib_close();
    return 0;
  } 
  fprintf(stderr,"Failed to initialize library\n");
  return 1;
}
