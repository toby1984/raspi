#ifndef MYLIB_H
#define MYLIB_H

#include "ui.h"

int mylib_add_button(char *text,int x,int y,int width,int height,ButtonHandler clickHandler);

int mylib_init(void);

void mylib_close(void);

#endif
