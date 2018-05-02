#include "log.h"
#include <stdio.h>
#include <stdarg.h>

void log_all(const char*level,char *msg,...) 
{
    char buffer[1024];
    
    buffer[0]=0;
    int len = snprintf(&buffer[0],1024,"%s: ",level);
    
    //        int vsnprintf(char *str, size_t size, const char *format, va_list ap);
    va_list ap;
    va_start(ap, msg); //Requires the last fixed parameter (to get the address)
    vsnprintf(&buffer[len], sizeof(buffer)-len-1, msg, ap);
    va_end(ap);
    printf("%s\n",&buffer[0]);
}

void log_info(char *msg,...) 
{
    va_list ap;
    va_start(ap, msg); //Requires the last fixed parameter (to get the address)    
    log_all("INFO",msg,ap);
    va_end(ap);
}

void log_debug(char *msg,...) {
    va_list ap;
    va_start(ap, msg); //Requires the last fixed parameter (to get the address)    
    log_all("DEBUG",msg,ap);
    va_end(ap);
}

void log_warn(char *msg,...) {
    va_list ap;
    va_start(ap, msg); //Requires the last fixed parameter (to get the address)    
    log_all("WARN",msg,ap);
    va_end(ap);    
}

void log_error(char *msg,...) {
    va_list ap;
    va_start(ap, msg); //Requires the last fixed parameter (to get the address)    
    log_all("ERROR",msg,ap);
    va_end(ap);    
}
