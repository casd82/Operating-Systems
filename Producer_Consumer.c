#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 5
#define MAX_SLEEP 5

typedef int buffer_item;

sem_t *sem_consume;
sem_t *sem_produce;
pthread_mutex_t mutex;

buffer_item buffer[BUFFER_SIZE];
int head = 0;
int tail = 0;
int count = 0;

void sleep_rand() {
	sleep(rand() % MAX_SLEEP + 1);
}

int insert_item(buffer_item item) {
	sem_wait(sem_produce);

	pthread_mutex_lock(&mutex);

	if (count == BUFFER_SIZE) return -1;

	buffer[tail] = item;
	tail = (tail + 1) % BUFFER_SIZE;
	count++;

	pthread_mutex_unlock(&mutex);

	sem_post(sem_consume);

	return 0;
}

int remove_item(buffer_item *item) {
	sem_wait(sem_consume);

	pthread_mutex_lock(&mutex);

	if (count == 0) return -1;
	
	*item = buffer[head];
	head = (head + 1) % BUFFER_SIZE;
	count--;

	pthread_mutex_unlock(&mutex);

	sem_post(sem_produce);

	return 0;
}

void *producer(void *param) {
	buffer_item item;

	while (1) {
		/* sleep for a random period of time */
		sleep_rand();

		/* generate a random number */
		item = rand();
		if (insert_item(item)) {
			fprintf(stderr, "report error condition\n");
		} else {
			printf("producer produced %d\n", item);
		}
	}
}

void *consumer(void *param) {
	buffer_item item;

	while (1) {
		/* sleep for a random period of time */
		sleep_rand();

		if (remove_item(&item)) {
			fprintf(stderr, "report error condition\n");
		} else {
			printf("consumer consumed %d\n", item);
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Should have 3 args:\n[1] The sleep time before terminate.\n[2] The number of producer threads.\n[3] The number of consumer threads.\n");
		return 1;
	}

	srand(time(NULL));

	/* get args */
	int sleep_time = atoi(argv[1]);
	int producer_count = atoi(argv[2]);
	int consumer_count = atoi(argv[3]);

	/* init threads */
	pthread_t *producer_threads = (pthread_t *) malloc(sizeof(pthread_t) * producer_count);
	pthread_t *consumer_threads = (pthread_t *) malloc(sizeof(pthread_t) * consumer_count);

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	/* init semaphore */
	if ((sem_produce = sem_open("/semaphore1", O_CREAT, 0644, BUFFER_SIZE)) == SEM_FAILED ||
		 (sem_consume = sem_open("/semaphore2", O_CREAT, 0644, 0)) == SEM_FAILED) {
		fprintf(stderr, "Error opening semaphore\n");
		return 2;
	}

	/* init mutex */
	pthread_mutex_init(&mutex, NULL);

	/* start threads */
	int i;
	for (i = 0; i < producer_count; i++) {
		pthread_create(&producer_threads[i], &attr, producer, NULL);
	}

	for (i = 0; i < consumer_count; i++) {
		pthread_create(&consumer_threads[i], &attr, consumer, NULL);
	}

	sleep(sleep_time);

	/* close semaphore */
	if (sem_close(sem_produce) == -1 || sem_close(sem_consume) == -1) {
		fprintf(stderr, "Error closing semaphore\n");
		return 3;
	}

	if (sem_unlink("/semaphore1") == -1 || sem_unlink("/semaphore2") == -1) {
		fprintf(stderr, "Error unlinking semaphore\n");
		return 4;
	}

	/* free memory */
	free(producer_threads);
	free(consumer_threads);

	/* Exit */
	printf("Done!\n");

	return 0;
}