#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define DOT_COUNT 5000

int in_circle = 0;

pthread_mutex_t mutex;

void *generate_dot(void *param) {
	double x = (double) rand() / (double) RAND_MAX;
	double y = (double) rand() / (double) RAND_MAX;

	if (x * x + y * y <= 1.0) {
		pthread_mutex_lock(&mutex);
		in_circle++;
		pthread_mutex_unlock(&mutex);
	}

	pthread_exit(0);
}

int main(void) {
	srand(time(NULL));

	pthread_mutex_init(&mutex, NULL);

	pthread_attr_t attr;
	pthread_t tid[DOT_COUNT];

	pthread_attr_init(&attr);

	int i;
	for (i = 0; i < DOT_COUNT; i++) {
		pthread_create(&tid[i], &attr, generate_dot, NULL);
	}	

	for (i = 0; i < DOT_COUNT; i++) {
		pthread_join(tid[i], NULL);
	}

	printf("PI is approx.: %f", (double) in_circle / (double) DOT_COUNT * 4.0);

	return 0;
}