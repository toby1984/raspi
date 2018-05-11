#ifndef GLOBAL_H
#define GLOBAL_H

#define ASSIGN_COLOR(color,red,green,blue) (color)->r=red; (color)->g=green; (color)->b=blue;

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
   
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })   
   
#endif