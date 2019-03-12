
/* ============================================================================
 * Introduction to Operating Systems
 * "Priority Readers and Writers" in Problem Set 1.
 *
 * KHALED AHMED ABDEL_GALIL
============================================================================ */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* CONSTANTS in systems */
#define READERS_IN_SYSTEM 5
#define NUMBER_OF_READS 5
#define WRITERS_IN_SYSTEM 5
#define NUMBER_OF_WRITES 5

/* GLOBAL SHARED DATA IN SYSTEM */
unsigned int gSharedValue = 0;
pthread_mutex_t gSharedMemoryLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gReadPhase = PTHREAD_COND_INITIALIZER;
pthread_cond_t gWritePhase = PTHREAD_COND_INITIALIZER;
int gWaitingReaders = 0, gReaders = 0;
int main(int argc, char **argv);
void *readerMain(void *threadArgument);
void *writerMain(void *threadArgument);

int main(int argc, char **argv) {

	int i;

	int readerNum[READERS_IN_SYSTEM];
	int writerNum[WRITERS_IN_SYSTEM];

	pthread_t readerThreadIDs[READERS_IN_SYSTEM];
	pthread_t writerThreadIDs[WRITERS_IN_SYSTEM];

	// Seed the random number generator
	srand((unsigned int)time(NULL));

	// Start the readers
	for(i = 0; i < READERS_IN_SYSTEM; i++) {
		readerNum[i] = i;
		pthread_create(&readerThreadIDs[i], NULL, readerMain, &readerNum[i]);
	}

	// Start the writers
	for(i = 0; i < WRITERS_IN_SYSTEM; i++) {
		writerNum[i] = i;
		pthread_create(&writerThreadIDs[i], NULL, writerMain, &writerNum[i]);
	}

	// Wait on readers to finish
	for(i = 0; i < READERS_IN_SYSTEM; i++) {
		pthread_join(readerThreadIDs[i], NULL);
	}

	// Wait on writers to finish
	for(i = 0; i < WRITERS_IN_SYSTEM; i++) {
		pthread_join(writerThreadIDs[i], NULL);
	}

  return 0;
}

void *readerMain(void *threadArgument) {

	int id = *((int*)threadArgument);
	int i = 0, numReaders = 0;

	for(i = 0; i < NUMBER_OF_READS; i++) {
		// Wait so that reads and writes do not all happen at once
	  usleep(1000 * (rand() % READERS_IN_SYSTEM + WRITERS_IN_SYSTEM));

		// Enter critical section
	  pthread_mutex_lock(&gSharedMemoryLock);
	  	gWaitingReaders++;
	  	while (gReaders == -1) {
	  		pthread_cond_wait(&gReadPhase, &gSharedMemoryLock);
	  	}
	  	gWaitingReaders--;
	  	numReaders = ++gReaders;
	  pthread_mutex_unlock(&gSharedMemoryLock);

	  // Read data
	  fprintf(stdout, "[r%d] reading %u  [readers: %2d]\n", id, gSharedValue, numReaders);

	  // Exit critical section
	  pthread_mutex_lock(&gSharedMemoryLock);
	  	gReaders--;
	  	if (gReaders == 0) {
	  		pthread_cond_signal(&gWritePhase);
	  	}
	  pthread_mutex_unlock(&gSharedMemoryLock);
	}

	pthread_exit(0);
}

void *writerMain(void *threadArgument) {

	int id = *((int*)threadArgument);
	int i = 0, numReaders = 0;

	for(i = 0; i < WRITERS_IN_SYSTEM; i++) {
	  // Wait so that reads and writes do not all happen at once
	  usleep(1000 * (rand() % READERS_IN_SYSTEM + WRITERS_IN_SYSTEM));

		// Enter critical section
	  pthread_mutex_lock(&gSharedMemoryLock);
	  	while (gReaders != 0) {
	  		pthread_cond_wait(&gWritePhase, &gSharedMemoryLock);
	  	}
	  	gReaders = -1;
	  	numReaders = gReaders;
	  pthread_mutex_unlock(&gSharedMemoryLock);

	  // Write data
	  fprintf(stdout, "[w%d] writing %u* [readers: %2d]\n", id, ++gSharedValue, numReaders);

	  // Exit critical section
	  pthread_mutex_lock(&gSharedMemoryLock);
	  	gReaders = 0;
	  	if (gWaitingReaders > 0) {
	  		pthread_cond_broadcast(&gReadPhase);
	  	}
	  	else {
	  		pthread_cond_signal(&gWritePhase);
	  	}
	  pthread_mutex_unlock(&gSharedMemoryLock);
  }

  pthread_exit(0);
}