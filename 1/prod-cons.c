#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "mt19937ar.h"

struct data {
	int value;
	int hold_time;
};

#define BUFFER_SIZE 32
struct data items[BUFFER_SIZE];
pthread_mutex_t mutex;
sem_t full, empty;
int counter;

int isx86();
int rand_num(int, int);
void *consumer(void *);
void *producer(void *);

int isx86() {
	unsigned int eax = 0x01, ebx, ecx, edx;
	
	__asm__ __volatile__(
			"cpuid;"
			: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
			: "a"(eax)
			);

	if (ecx & 0x40000000) { return 1; }
	
	return 0;
}

int rand_num(int max, int min) {
	int result = 0;

	if (isx86()) {
		__asm__ __volatile__(
				"rdrand %0":"=r"(result)
				);
	} else {
		result = (int)genrand_int32();
	}

	result = abs(result % (max-min));
	if (result < min) {
		return min;
	}

	return result;
}

void *producer(void *args) {
	while(1) {
		int hold_time = rand_num(9, 2);
		sleep(rand_num(7, 3));

		int value = rand_num(INT_MAX, 0);

		sem_wait(&empty);

		pthread_mutex_lock(&mutex);
		if (counter < BUFFER_SIZE){
			printf("producer: produce value %d\n", value);
			items[counter].value = value;
			items[counter].hold_time = hold_time;
			counter++;
		}
		pthread_mutex_unlock(&mutex);

		sem_post(&full);
	}
}

void *consumer(void *args) {
	while(1) {
		sleep(items[counter].hold_time);

		sem_wait(&full);

		pthread_mutex_lock(&mutex);
		if (counter < BUFFER_SIZE) {
			printf("consumer: consume value %d\n", items[counter].value);
			counter--;
		}
		pthread_mutex_unlock(&mutex);
		
		sem_post(&empty);
	}	
}

int main(int argc, char* argv[]) {
	counter = 0;
	
	if (argc < 1) {
		printf("Usage: ./a.out [number of threads]");
	}
	int threads = atoi(argv[1]);
	isx86();

	pthread_t thread_array[2*threads];
	
	for (int i=0; i<threads; i++) {
		pthread_create(&thread_array[i], NULL, consumer, NULL);
		pthread_create(&thread_array[i+1], NULL, producer, NULL);
	}

	for (int i=0; i<(2*threads); i++) {
		pthread_join(thread_array[i], NULL);
	}

	return 0;
}
