#include <Python.h>
#include "mylib.h"
#include <stdlib.h>

typedef struct callback_entry 
{
  struct callback_entry *next;
  int buttonId;
  PyObject *clickHandler;
} callback_entry;

static callback_entry *handlers = NULL;

static void call_python2(PyObject *buttonCallback,int buttonId) 
{
  PyObject *arglist = Py_BuildValue("(i)", buttonId); // need to use '(i)' and not just 'i' as PyEval_CallObject() requires a tuple
  PyObject *result = PyEval_CallObject(buttonCallback, arglist);      
  Py_DECREF(arglist);
  if ( result != NULL ) {
    Py_DECREF(result);
  }    
}

// aquires the GIL if necessary and then proceeds to invoke the actual callback
static void call_python(PyObject *buttonCallback,int buttonId) 
{

  PyThreadState * currentThread = _PyThreadState_Current;
  if (PyGILState_GetThisThreadState() || !currentThread)
  {
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    call_python2(buttonCallback,buttonId);
    
    PyGILState_Release(gstate);
  }
  else
  {
    call_python2(buttonCallback,buttonId);
  }
}

static void myui_clickHandler(int buttonId) {
  
  callback_entry *current=handlers;
  while( current ) 
  {
    if ( current->buttonId == buttonId ) 
    {
      call_python(current->clickHandler,buttonId);
      break;
    }
    current = current->next;
  }
}

static void myui_free_callback_entry(callback_entry *entry) 
{
  Py_DECREF(entry->clickHandler);  
  free(entry);
}

static int myui_add_button_handler(int buttonId,PyObject *buttonCallback) 
{
  callback_entry *entry = malloc(sizeof(callback_entry));
  if ( ! entry ) {
    return 0;  
  }
  entry->buttonId = buttonId;
  
  Py_INCREF(buttonCallback);   
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
    // tell Python to enable the GIL, we'll need it
    // later when we're trying to call into the Python interpreter
    // from a background thread that was NOT created from inside Python 
    PyEval_InitThreads();  
    
    return PyInt_FromLong( mylib_init() );
}

static PyObject *myui_close(PyObject *self, PyObject *args) 
{
    mylib_close();
    
    callback_entry *current=handlers;
    while( current ) 
    {
      callback_entry *next=current->next;
      myui_free_callback_entry(current);
      current = next;
    }    
    return Py_None;   
}

static PyObject *myui_add_button(PyObject *self, PyObject *args)
{
    char *buttonText;
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
        call_python(callback,42);
        int buttonId = mylib_add_button(buttonText,buttonX,buttonY,buttonWidth,buttonHeight,myui_clickHandler);        
        if ( buttonId >= 0 ) 
        {
          myui_add_button_handler(buttonId,callback);
        }
        return PyInt_FromLong(buttonId);
    }
    return PyInt_FromLong(0);   
}

static PyObject *myui_add_image_button(PyObject *self, PyObject *args)
{
    char *imagePath;
    int buttonX;
    int buttonY;
    int buttonWidth;
    int buttonHeight;
    PyObject *callback;
    
    if (!PyArg_ParseTuple(args, "siiiiO", &imagePath,&buttonX,&buttonY,&buttonWidth,&buttonHeight,&callback)) {      
        return NULL;
    }
    
    // make sure the callback actually is a python function
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "Need a callback function to invoke the button!");
    }
    else 
    {
        call_python(callback,42);
        int buttonId = mylib_add_image_button(imagePath,buttonX,buttonY,buttonWidth,buttonHeight,myui_clickHandler);        
        if ( buttonId >= 0 ) 
        {
          myui_add_button_handler(buttonId,callback);
        }
        return PyInt_FromLong(buttonId);
    }
    return PyInt_FromLong(0);   
}

static PyMethodDef availableMethods[] = 
{
    {"init",  myui_init, METH_VARARGS,"Initialize library."},
    {"close",  myui_close, METH_VARARGS,"Close library."},
    {"add_button",  myui_add_button, METH_VARARGS,"Add a ui button"},
    {"add_image_button",  myui_add_image_button, METH_VARARGS,"Add a ui image button"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC inituilib(void)
{
    PyObject *m = Py_InitModule("uilib", availableMethods);
    if (m != NULL) {
    }
}
