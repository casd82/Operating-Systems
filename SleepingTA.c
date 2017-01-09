#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_SLEEP_TIME 10
#define MIN_SLEEP_TIME 1
#define SEATS 3
#define STUDENTS 10

pthread_mutex_t mutex;
sem_t std_sem, ta_sem;

int seats[SEATS];
int next_seat = 0;
int teaching_seat = 0;
int seats_available = SEATS;

void sleep_for_random_sec() {
	sleep((rand() % (MAX_SLEEP_TIME - MIN_SLEEP_TIME + 1)) + MIN_SLEEP_TIME);
}

void *student_thread(void *param) {
	int std_id = *((int *) param);
	printf("Student %d is programming...\n", std_id);

	sleep_for_random_sec();

	while (1) {
		/* go to the TA office */
		pthread_mutex_lock(&mutex);

		/* have seat */
		if (seats_available > 0) {
			seats_available--;

			/* sits down */
			seats[next_seat] = std_id;
			printf("Student %d sits down...\n", std_id);
			printf("Seat 1: %d, Seat 2: %d, Seat 3: %d\n", seats[0], seats[1], seats[2]);
			next_seat = (next_seat + 1) % SEATS; /* reset next seat */
			
			pthread_mutex_unlock(&mutex);

			/* notify TA */
			sem_post(&std_sem); 

			/* wait until the TA is available */
			sem_wait(&ta_sem); 

		} else {
			/* back to programming */
			pthread_mutex_unlock(&mutex);
			printf("No available seats... Student %d goes back to programming...\n", std_id);
			sleep_for_random_sec();
		}
	}

	pthread_exit(0);
}

void *ta_thread(void *param) {
	while (1) {
		/* wait for student to come */
		sem_wait(&std_sem); 

		pthread_mutex_lock(&mutex);

		seats_available++;

		int student_id = seats[teaching_seat];

		/* teaching */
		printf("Now teaching student %d\n", student_id);
		seats[teaching_seat] = 0;
		teaching_seat = (teaching_seat + 1) % SEATS;

		sleep_for_random_sec();

		printf("Finish teaching student %d\n", student_id);

		pthread_mutex_unlock(&mutex);

		/* check if there are students */
		sem_post(&ta_sem); 
	}
	pthread_exit(0);
}

int main(void) {
	srand(time(NULL));

	/* init pthread attribute */
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	/* init semaphores */
	sem_init(&std_sem, 0, 0); /* no student comes yet */
	sem_init(&ta_sem, 0, 1); /* 1 available space for teaching */

	/* init mutex */
	pthread_mutex_init(&mutex, NULL);

	/* TA starts working */
	pthread_t thread_TA, thread_students[STUDENTS];
	pthread_create(&thread_TA, &attr, ta_thread, NULL);

	int student_ids[STUDENTS];

	int i;
	/* students start programming */
	for (i = 0; i < STUDENTS; i++) {
		/* "i" is the id of the students */
		student_ids[i] = i + 1;
		pthread_create(&thread_students[i], &attr, student_thread, (void *) &student_ids[i]);
	}

	pthread_join(thread_TA, NULL);
	for (i = 0; i < STUDENTS; i++) {
		pthread_join(thread_students[i], NULL);
	}

	return 0;
}