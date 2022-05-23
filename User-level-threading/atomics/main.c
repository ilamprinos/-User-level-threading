#include "atomic.h"
#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4
#define TARGET 1000000000

atomic_t counter;
spin_t lock;

void *threadFuncNaive (void *arg) {
	int i;
	
	for (i = 0; i < TARGET/4; i++) {
		counter++;
	}		
	return NULL;
}

void *threadFuncLock (void *arg) {
	int i;
	
	for (i = 0; i < TARGET/4; i++) {
		spin_lock(&lock);
		counter++;
		spin_unlock(&lock);
	}	
	return NULL;
}

void *threadFuncAtomic (void *arg) {
	int i;
	
	for (i = 0; i < TARGET/4; i++) {
		atomic_add(1, &counter);
	}	
	return NULL;
}


int main(int argc, char *argv[]){
	int i; 
	pthread_t threadId[NUM_THREADS-1];

	
	atomic_set (0, &counter);
	
	for (i = 0; i < NUM_THREADS-1; i++) {
		pthread_create(&threadId[i], NULL, &threadFuncNaive, NULL);
	}
	threadFuncNaive(NULL);

	for (i = 0; i < NUM_THREADS-1; i++) {
		pthread_join(threadId[i], NULL);
	}

	printf("Counter with naive: %d\n", atomic_read(&counter));	


	
	atomic_set (0, &counter);
	
	for (i = 0; i < NUM_THREADS-1; i++) {
		pthread_create(&threadId[i], NULL, &threadFuncAtomic, NULL);
	}
	threadFuncAtomic(NULL);

	for (i = 0; i < NUM_THREADS-1; i++) {
		pthread_join(threadId[i], NULL);
	}

	printf("Counter with atomics: %d\n", atomic_read(&counter));


	spin_lock_init (&lock);
	atomic_set (0, &counter);
	for (i = 0; i < NUM_THREADS-1; i++) {
		pthread_create(&threadId[i], NULL, &threadFuncLock, NULL);
	}
	threadFuncLock(NULL);

	for (i = 0; i < NUM_THREADS-1; i++) {
		pthread_join(threadId[i], NULL);
	}
	
	printf("Counter with locks: %d\n", atomic_read(&counter));
	
	return 0;
}
