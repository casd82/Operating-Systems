#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define PHILOSOPHERS 5
#define MAX_SLEEP_TIME 10

pthread_mutex_t mutex;
pthread_cond_t cond_var;

enum {EATING, THINKING} philosophers[PHILOSOPHERS];

void pickup_forks(int philosopher_number) {
	printf("Philosopher %d is eating\n", philosopher_number);
	philosophers[philosopher_number] = EATING;
}

void return_forks(int philosopher_number) {
	printf("Philosopher %d returns the forks\n", philosopher_number);
	philosophers[philosopher_number] = THINKING;
}

void *philosopher_thread(void *param) {
	int id = *((int *) param);
	int right_neighbor = (id + 1) % PHILOSOPHERS;
	int left_neighbor = (id + PHILOSOPHERS - 1) % PHILOSOPHERS;

	while (1) {
		printf("Philosopher %d is thinking...\n", id);

		sleep(rand() % MAX_SLEEP_TIME + 1);

		/* hungry */
		pthread_mutex_lock(&mutex);

		while (philosophers[right_neighbor] == EATING || philosophers[left_neighbor] == EATING) {
			pthread_cond_wait(&cond_var, &mutex);
		}

		pickup_forks(id);

		pthread_mutex_unlock(&mutex);

		sleep(rand() % MAX_SLEEP_TIME + 1);

		pthread_mutex_lock(&mutex);

		return_forks(id);

		pthread_cond_signal(&cond_var);

		pthread_mutex_unlock(&mutex);

	}

	pthread_exit(0);
}

int main(void) {
	pthread_t philosopher_threads[PHILOSOPHERS];
	pthread_attr_t attr;

	pthread_attr_init(&attr);

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond_var, NULL);

	int philo_id[PHILOSOPHERS], i;
	for (i = 0; i < PHILOSOPHERS; i++) {
		philosophers[i] = THINKING;
		philo_id[i] = i;
		pthread_create(&philosopher_threads[i], &attr, philosopher_thread, (void *) &philo_id[i]);
	}

	for (i = 0; i < PHILOSOPHERS; i++) {
		pthread_join(philosopher_threads[i], NULL);
	}

	return 0;
}