#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SLEEP 8
#define NORTH_FARMERS 10
#define SOUTH_FARMERS 7

pthread_mutex_t mutex;

void sleep_rand(void) {
	sleep(rand() % MAX_SLEEP + 1);
}

void *northbound_farmer(void *param) {
	int id = *((int *) param);

	sleep_rand();

	printf("Farmer #N%d arrives at the bridge!\n", id);

	pthread_mutex_lock(&mutex);

	printf("Farmer #N%d starts passing the bridge...\n", id);

	sleep_rand();

	printf("Farmer #N%d exits the bridge...\n", id);

	pthread_mutex_unlock(&mutex);

	pthread_exit(0);
}

void *southbound_farmer(void *param) {
	int id = *((int *) param);

	sleep_rand();

	printf("Farmer #S%d arrives at the bridge!\n", id);

	pthread_mutex_lock(&mutex);

	printf("Farmer #S%d starts passing the bridge...\n", id);

	sleep_rand();

	printf("Farmer #S%d exits the bridge...\n", id);

	pthread_mutex_unlock(&mutex);

	pthread_exit(0);
}

int main(void) {
	srand(time(NULL));

	/* init mutex */
	pthread_mutex_init(&mutex, NULL);

	/* init pthreads */
	pthread_attr_t attr;
	pthread_t north_farmer_threads[NORTH_FARMERS], south_farmer_threads[SOUTH_FARMERS];
	pthread_attr_init(&attr);

	/* farmer id */
	int north_id[NORTH_FARMERS], south_id[SOUTH_FARMERS], i;

	/* start pthreads */
	for (i = 0; i < NORTH_FARMERS; i++) {
		north_id[i] = i + 1;
		pthread_create(&north_farmer_threads[i], &attr, northbound_farmer, (void *) &north_id[i]);
	}

	for (i = 0; i < SOUTH_FARMERS; i++) {
		south_id[i] = i + 1;
		pthread_create(&south_farmer_threads[i], &attr, southbound_farmer, (void *) &south_id[i]);
	}

	/* wait for threads to finish */
	for (i = 0; i < NORTH_FARMERS; i++) {
		pthread_join(north_farmer_threads[i], NULL);
	}

	for (i = 0; i < SOUTH_FARMERS; i++) {
		pthread_join(south_farmer_threads[i], NULL);
	}

	printf("Finished!\n");

	return 0;
}