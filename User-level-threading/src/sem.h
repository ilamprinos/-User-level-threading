#ifndef SEM_H
#define SEM_H
#include <semaphore.h>
// #include<pthread.h>

// Setting max value to 1 keeps the semaphores as binary
#define SEM_MAX_VALUE 1

int sem_init_(sem_t *s,int val);

int sem_down(sem_t *s);

int sem_up(sem_t* s);

int sem_destroy_(sem_t *s);

#endif