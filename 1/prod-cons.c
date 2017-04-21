#include <stdio.h>
#include <pthread.h>
#include "mt19937ar.h"

struct data {
	int value;
	int hold_time;
};

bool isx86();
int rand_num(int, int);
void consumer();
void producer();

bool isx86() {
	unsigned int eax = 0x01, ebc, ecx, edx;
	
	__asm__ __volatile__(
			"cpuid;"
			: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
			: "a"(eax)
			);

	if (ecx & 0x40000000) { return true; }
	
	return false;
}

int rand_num(int max, int min) {
	int result = 0;

	if (isx86()) {
		__asm__ __volatile__(
				"rdrand %0":"=r"(result)
				);
	}

	result = abs(result % (max-min));
	if (result < min) {
		return min;
	}

	return result;
}

int main(int argc, char* argv[]) {
	int threads, i;

	if (argc < 1) {
		printf("Usage: ./a.out [number of threads]");
	}
	threads = atoi(argv[1]);

	

	pthread_t consumer_process;
	pthread_create(&consumer_process, NULL, (void *)consumer, (void *)NULL);
}
