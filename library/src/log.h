#ifndef LOG_H
#define LOG_H

enum LogLevel { ERROR=0,WARN=1,INFO=2,DEBUG=3,TRACE=4 };

void log_info(char *msg,...);
void log_debug(char *msg,...);
void log_warn(char *msg,...);
void log_error(char *msg,...);

#endif
