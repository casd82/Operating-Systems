/* a long and messy bankers algorithm with simulation using pthreads */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAX_SLEEP 5

/* these may be any values >= 0 */
#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

/* the available amount of each resource */
int available[NUMBER_OF_RESOURCES];

/* the maximum demand of each customer */
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

/* the amount currently allocated to each customer */
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

/* the remaining need of each customer */
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

pthread_mutex_t mutex;

void sleep_rand(void) {
	sleep(rand() % MAX_SLEEP + 1);
}

int is_safe(void) {
	int work[NUMBER_OF_RESOURCES], finish[NUMBER_OF_CUSTOMERS], i, j;
	for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
		work[i] = available[i];
	}

	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		finish[i] = 0;
	}

	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		int leseq_to_work = 1;
		for (j = 0; j < NUMBER_OF_RESOURCES; j++) {
			if (need[i][j] > work[j]) {
				leseq_to_work = 0;
				break;
			}
		}

		if (!finish[i] && leseq_to_work) {
			for (j = 0; j < NUMBER_OF_RESOURCES; j++) {
				work[j] += allocation[i][j];
				finish[i] = 1;
				i = 0;
			}
		}
	}

	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		if (!finish[i]) {
			return 0;
		}
	}

	return 1;
}

int request_resources(int customer_num, int request[]) {
	pthread_mutex_lock(&mutex);
	int i;
	for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
		if (request[i] > need[customer_num][i]) {
			printf("Exceeded maximum claim!\n");
			pthread_mutex_unlock(&mutex);
			return -1;
		}
	}

	for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
		if (request[i] > available[i]) {
			printf("Customer %d should wait...\n", customer_num);
			pthread_mutex_unlock(&mutex);
			return -1;
		}
	}

	for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
		available[i] -= request[i];
		allocation[customer_num][i] += request[i];
		need[customer_num][i] -= request[i];
	}

	if (is_safe()) {
		printf("Customer %d completed a transaction!\n", customer_num);
		pthread_mutex_unlock(&mutex);
		return 1;
	} else {
		printf("Customer %d requested to an unsafe state, thus is denied\n", customer_num);
		for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
			available[i] += request[i];
			allocation[customer_num][i] -= request[i];
			need[customer_num][i] += request[i];
		}
		pthread_mutex_unlock(&mutex);
		return -1;
	}
}

int release_resources(int customer_num, int release[]) {
	int i;
	pthread_mutex_lock(&mutex);
	for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
		allocation[customer_num][i] -= release[i];
		available[i] += release[i];

		if (allocation[customer_num][i] < 0) {
			return -1;
		}
	}
	pthread_mutex_unlock(&mutex);
	return 1;
}

/* check if arr is all zero */
int is_all_zero(int len, int * arr) {
	int i;
	for (i = 0; i < len; i++) {
		if (arr[i] != 0) {
			return 0;
		}
	}

	return 1;
}

void *customer(void *param) {
	int id = *((int *) param);
	int resource[NUMBER_OF_RESOURCES];

	while (1) {
		/* check if the customer still needs any resource */
		if (is_all_zero(NUMBER_OF_RESOURCES, need[id])) {
			break;
		}

		sleep_rand();

		/* request resource */
		int i;
		for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
			if (need[id][i] == 0) {
				resource[i] = 0;
				continue;
			}
			resource[i] = rand() % need[id][i] + 1;
		}
		if (request_resources(id, resource) == -1) {
			continue;
		}

		sleep_rand();

		if (release_resources(id, resource) == -1) {
			fprintf(stderr, "Error Occur While Releasing Resources!\n");
			pthread_exit(0);
		}

	}

	printf("Customer #%d Done.\n", id);

	pthread_exit(0);
}

int main(int argc, char *argv[]) {
	if (argc != NUMBER_OF_RESOURCES + 1) {
		fprintf(stderr, "You should enter %d integers indicating the instances of resource types.\n", NUMBER_OF_RESOURCES);
		return 1;
	}

	srand(time(NULL));

	int i, j;

	/* init available array */
	for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
		available[i] = atoi(argv[i+1]);
	}

	/* init max, need, allocation */
	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		printf("Enter the maximum resource required for customer #%d:\n", i);

		for (j = 0; j < NUMBER_OF_RESOURCES; j++) {
			scanf("%d", &maximum[i][j]);
			need[i][j] = maximum[i][j];
			allocation[i][j] = 0;
		}
	}

	/* init pthread & mutex */
	pthread_mutex_init(&mutex, NULL);

	pthread_t tid[NUMBER_OF_CUSTOMERS];
	pthread_attr_t attr;
	int customer_id[NUMBER_OF_CUSTOMERS];

	pthread_attr_init(&attr);

	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		customer_id[i] = i;
		pthread_create(&tid[i], &attr, customer, (void *) &customer_id[i]);
	}

	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		pthread_join(tid[i], NULL);
	}

	printf("DONE!\n");

	return 0;
}