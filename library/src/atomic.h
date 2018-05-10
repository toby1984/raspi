#ifndef ATOMIC_H
#define ATOMIC_H
#include <pthread.h>

// wait for condition
#define render_wait_condition(mutex,condition) \
      pthread_mutex_lock(mutex); \
      pthread_cond_wait(condition, mutex); \
      pthread_mutex_unlock(mutex); 
      
// signal condition
#define render_signal_condition(mutex,condition) \
    pthread_mutex_lock(mutex); \
    pthread_cond_signal(condition); \
    pthread_mutex_unlock(mutex);  
    
#define render_init_condition(mutex,condition) \
      pthread_mutex_init(mutex, NULL); \
      pthread_cond_init(condition,NULL);
      
#endif      
