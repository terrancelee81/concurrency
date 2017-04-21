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
		int sleep_time = rand_num(7, 3);
		sleep(sleep_time);
		int value = rand_num(50000, 1);

		sem_wait(&empty);
				
		if (counter < BUFFER_SIZE){
			pthread_mutex_lock(&mutex);
			printf("producer: produce value %d\n", value);
			items[counter].value = value;
			items[counter].hold_time = hold_time;
			//if (counter == 31) { counter = 0; }
			counter++;
		} else { pthread_mutex_unlock(&mutex); }

		sem_post(&full);
	}
}

void *consumer(void *args) {
	while(1) {
		sleep(items[counter].hold_time);
		
		sem_wait(&full);

		if (counter < BUFFER_SIZE /*&& items[counter].value != 0*/) {
			pthread_mutex_lock(&mutex);
			printf("consumer: consume value %d\n", items[counter].value);
//			if (counter == 31) { counter = 0; }
//			counter--;
		} else { pthread_mutex_unlock(&mutex); }
		sem_post(&empty);
	}	
}

int main(int argc, char* argv[]) {
	counter = 0;
	pthread_t id;
	pthread_attr_t attr;
	pthread_mutex_init(&mutex, NULL);
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, BUFFER_SIZE);
	pthread_attr_init(&attr);
	
	if (argc < 3) {
		printf("Usage: ./a.out [number of producer threads] [number of consumer threads]\n");
		return 0;
	}
	int prod_threads = atoi(argv[1]);
	int cons_threads = atoi(argv[2]);
	isx86();

	for (int i=0; i<prod_threads; i++) {
		pthread_create(&id, &attr, producer, NULL);
	}
	printf("producer threads created ... \n");

	for (int i=0; i<cons_threads; i++) {
		pthread_create(&id, &attr, consumer, NULL);
	}
	printf("consumer threads created ... \n");	
	sleep(100);	
//	for (int i=0; i<(2*threads); i++) {
//		pthread_join(id, NULL);
//	}

//	printf("threads joined ... \n");

	return 0;
}
