#include "log.h"
#include <stdio.h>
#include <stdarg.h>

static enum LogLevel level = INFO;

void log_all(const char*level,char *msg,va_list ap) 
{
    char buffer[1024];
    
    buffer[0]=0;
    int len = snprintf(&buffer[0],1024,"%s: ",level);    
    vsnprintf(&buffer[len], sizeof(buffer)-len-1, msg, ap);
    printf("%s\n",&buffer[0]);
}

void log_info(char *msg,...) 
{
    if ( level >= INFO ) {
        va_list ap;
        va_start(ap, msg); 
        log_all("INFO",msg,ap);
        va_end(ap);
    }
}

void log_debug(char *msg,...) {
    if ( level >= DEBUG ) {
        va_list ap;
        va_start(ap, msg); 
        log_all("DEBUG",msg,ap);
        va_end(ap);
    }
}

void log_warn(char *msg,...) {
    if ( level >= WARN ) {
        va_list ap;
        va_start(ap, msg); 
        log_all("WARN",msg,ap);
        va_end(ap);    
    }
}

void log_error(char *msg,...) {
    if ( level >= ERROR ) {    
        va_list ap;
        va_start(ap, msg); 
        log_all("ERROR",msg,ap);
        va_end(ap);    
    }
}
