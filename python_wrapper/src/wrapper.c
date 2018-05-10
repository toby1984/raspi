#include <Python.h>
#include "mylib.h"
#include <stdlib.h>

typedef struct callback_entry 
{
  struct callback_entry *next;
  int buttonId;
  PyObject *clickHandler;
} callback_entry;

callback_entry *handlers = NULL;

static void clickHandler(int buttonId) {
  
  callback_entry *current=handlers;
  while( current ) 
  {
    if ( current->buttonId == buttonId ) 
    {
      PyObject *arglist = Py_BuildValue("(i)", buttonId);
      PyObject *result = PyEval_CallObject(current->clickHandler, arglist);        
      break;
    }
  }
}

static int add_button_handler(int buttonId,PyObject *buttonCallback) {
  callback_entry *entry = malloc(sizeof(callback_entry));
  if ( ! entry ) {
    return 0;  
  }
  entry->buttonId = buttonId;
  entry->clickHandler = buttonCallback;
  
  if ( handlers == NULL ) {
    handlers = entry;  
    entry->next=NULL;
  } else {
    entry->next=handlers;
    handlers=entry;
  }
  return 1;
}

static PyObject *myui_init(PyObject *self, PyObject *args) 
{
    return PyInt_FromLong( mylib_init() );
}

static PyObject *myui_close(PyObject *self, PyObject *args) 
{
    mylib_close();
    return Py_None;   
}

static PyObject *myui_add_button(PyObject *self, PyObject *args)
{
  // int mylib_add_button(char *text,int x,int y,int width,int height,ButtonHandler clickHandler);
  
    const char *buttonText;
    int buttonX;
    int buttonY;
    int buttonWidth;
    int buttonHeight;
    PyObject *callback;

    if (!PyArg_ParseTuple(args, "siiiiO", &buttonText,&buttonX,&buttonY,&buttonWidth,&buttonHeight,&callback)) {
        return NULL;
    }
    
    // make sure the callback actually is a python function
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "Need a callback function to invoke the button!");
    }
    else 
    {
        int buttonId = mylib_add_button(buttonText,buttonX,buttonY,buttonWidth,buttonHeight,clickHandler);        
        if ( buttonId >= 0 ) 
        {
          Py_INCREF(callback);          
          add_button_handler(buttonId,callback);
        }
        return PyInt_FromLong(buttonId);
    }
    // Py_INCREF(Py_None);
    return Py_None;    
}

static PyMethodDef availableMethods[] = 
{
    {"init",  myui_init, METH_VARARGS,"Execute a shell command."},
    {"close",  myui_close, METH_VARARGS,"Execute a shell command."},
    {"system",  myui_add_button, METH_VARARGS,"Execute a shell command."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC inituilib(void)
{
    PyObject *m = Py_InitModule("myui", availableMethods);
    if (m != NULL) {
    }
}
